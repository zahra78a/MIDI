﻿using MidiService.Model;
using System;
using System.Collections.Generic;
using System.ComponentModel;
using System.IO.Pipes;
using System.Linq;
using System.Linq.Expressions;
using System.Security.AccessControl;
using System.Security.Principal;
using System.Text;
using System.Threading.Tasks;

namespace MidiService.Services
{
    public abstract class PipeProcessingServiceBase : BackgroundService
    {
        protected readonly ILogger _logger;
        private readonly IHostApplicationLifetime _lifetime;

        protected NamedPipeServerStream? _pipe;

        protected string PipeName { get; set; }

        public PipeProcessingServiceBase(
            ILogger logger,
            IHostApplicationLifetime lifetime) =>
            (_logger, _lifetime) = (logger, lifetime);



        private void CreatePipe()
        {
//            _logger.LogDebug($"DEBUG: Creating pipe {PipeName}");

            try
            {
                SecurityIdentifier securityIdentifier = new SecurityIdentifier(
                    WellKnownSidType.AuthenticatedUserSid, null);

                PipeSecurity security = new PipeSecurity();
                PipeAccessRule accessRule = new PipeAccessRule(
                    securityIdentifier,
                    PipeAccessRights.ReadWrite, AccessControlType.Allow);

                security.AddAccessRule(accessRule);

                _pipe = NamedPipeServerStreamAcl.Create(
                            PipeName,
                            PipeDirection.InOut,
                            1,
                            PipeTransmissionMode.Message,
                            PipeOptions.None,
                            2048, 2048,
                            security);

 //               _logger.LogDebug($"DEBUG: Created pipe server stream {PipeName}");
            }
            catch (InvalidOperationException ex)
            {
                // already established
                _logger.LogError($"{PipeName} Pipe already established. " + ex.Message);
            }
            catch (Exception ex)
            {
                _logger.LogError($"Could not create pipe {PipeName}. This is sometimes because of a quick restart when Windows hasn't yet removed the old pipe instance. Exception: " + ex.Message);
                throw;
            }
        }



        private void WaitForPipeConnection(CancellationToken stoppingToken)
        {
                try
                {
                    if (_pipe == null)
                    {
                        _logger.LogError("WaitForPipeConnection: Pipe is null.");
                        throw new Exception("Pipe is null in WaitForPipeConnection.");
                    }

                    while (!stoppingToken.IsCancellationRequested)
                    {
                        try
                        {
                            _logger.LogInformation($"Waiting for connection on pipe {PipeName}.");

                            // This will throw TaskCanceledException if service is shut down
                            // It will throw IOException if the client disconnected (this is normal)

                            //await _pipe.WaitForConnectionAsync(stoppingToken);
                            _pipe.WaitForConnection();

                            //_logger.LogDebug($"DEBUG: Connection established on pipe {PipeName}. Returning from WaitForPipeConnection");

                            return;

                        }
                        catch (IOException)
                        {
                            // client disconnected. This is normal. Restart the loop to wait
                            // for the next client to connect
                            //_logger.LogDebug("Client disconnected. Restarting loop.");

                            _pipe.Disconnect();
                            continue;       // restart the loop. Hey, it's almost as good as a GOTO
                        }
                    }

                    //_logger.LogDebug("DEBUG: Exited loop. Returning from WaitForPipeConnection.");

                }
                catch (TaskCanceledException)
                {
                    _logger.LogInformation("Task Canceled: WaitForPipeConnection.");

                    if (_pipe != null && _pipe.IsConnected)
                    {
                        _pipe.Disconnect();
                    }
                    throw;
                }
                catch (Exception ex)
                {
                    _logger.LogError("Exception in WaitForPipeConnection");

                    throw new Exception("Exception waiting for pipe connection. ", ex);
                }
        }




        public override Task StartAsync(CancellationToken cancellationToken)
        {
            //_logger.LogInformation("DEBUG: StartAsync");

            return base.StartAsync(cancellationToken);
        }

        public override Task StopAsync(CancellationToken cancellationToken)
        {
           // _logger.LogInformation("DEBUG: StopAsync");

            if (_pipe != null)
            {
                _logger.LogInformation("Stopping service. Closing pipe.");

                if (_pipe.IsConnected)
                {
                    _pipe.Disconnect();
                }

                _pipe.Dispose();
                _pipe = null;
            }
            else
            {
                _logger.LogInformation("Stopping service. Pipe was not open.");
            }

            return base.StopAsync(cancellationToken);
        }


        protected abstract void OnConnectionEstablished(CancellationToken stoppingToken);

        protected override async Task ExecuteAsync(CancellationToken stoppingToken)
        {
            //_logger.LogInformation("DEBUG: ExecuteAsync");

            // I wanted to do this in StartAsync, but ExecuteAsync can start before StartAsync completes.
            CreatePipe();

            if (_pipe == null)
            {
                throw new Exception("Could not start service. Pipe is null.");
            }



            try
            {
                // main service loop
                while (!stoppingToken.IsCancellationRequested)
                {
                    WaitForPipeConnection(stoppingToken);

                    OnConnectionEstablished(stoppingToken);

                    // need to prevent a super tight loop for this to allow SCM to manage it
                    // a 50ms delay in THIS loop is not the end of the world. We 
                    // wouldn't want anything like this in the MIDI message processing
                    // loops, however.
                    await Task.Delay(50);
                }
            }
            catch (TaskCanceledException)
            {
                _logger.LogWarning("Service Task canceled at {time}", DateTimeOffset.Now);

                // Need to shut everything down

                _pipe.Disconnect();
                _pipe.Dispose();
            }
            catch (Exception ex)
            {
                _logger.LogError("Fatal exception. Shutting down service. " + ex.ToString());

                _lifetime.StopApplication();
            }
        }
    }
}
