% PupilEXT Remote Control Connection helper class for Matlab using Psychtoolbox 3
% Compatible with the PupilEXT software (currently v0.1.2 beta)
% Author: Gabor Benyei
% License: GNU GPL v3 (identical to that of the PupilEXT software, see "Misc/LICENSE")
% Note: This code was tested to work with Matlab R2020b

classdef PupilEXT 
   
   % NOTE: These are just internal defaults, you do not need to change them
   properties
      Version = 1
      Enabled = false
      Method = 0
      UDP_IP = '192.168.40.1'
      UDP_Port = 6900
      UDP_Conn
      COM_Port = 'COM1'
      COM_BaudRate = 9600
      COM_ByteOrder = 'little-endian'
      COM_FlowControl = 'none'
      COM_StopBits = 1
      COM_DataBits = 8
      COM_Parity = 'none'
      COM_Timeout = 10
      COM_Terminator = 'LF'
      COM_Conn
      RecordingsPath = 'C:/PupilEXT_Recordings'
      ParticipantName = '1234'
      DataRecordingDelimiter = ';'
      ImageRecordingFormat = 'tiff'
   end
   
   %%
   
   methods (Access = private)
      
      function printVersion(obj)
          disp(['Using PupilEXT Remote Control Connection helper class VERSION ' num2str(obj.Version) ' for Matlab R2019a and prior versions, using Psychtoolbox 3.']);
      end
      
      function obj = setupConnectionLayer(obj)
          try
              if obj.Method == 0
                  obj.UDP_Conn = pnet('udpsocket', obj.UDP_Port);
              else 
                  obj.COM_Conn = serialport(obj.COM_Port, obj.COM_BaudRate, ...
                      'ByteOrder',obj.COM_ByteOrder, ...
                      'FlowControl',obj.COM_FlowControl, ...
                      'StopBits',obj.COM_StopBits, ...
                      'DataBits',obj.COM_DataBits, ...
                      'Parity',obj.COM_Parity, ...
                      'Timeout',obj.COM_Timeout );
                  configureTerminator(obj.COM_Conn, obj.COM_Terminator)
              end
          catch e
              fprintf(1,'Error message:\n%s',e.message);
              error('Could not setup connection to PupilEXT host computer.');
          end
      end
      
      function obj = freeConnectionLayer(obj)
          if obj.Method == 0
            pnet(obj.UDP_Conn,'close');
            obj.UDP_Conn = [];
            clear obj.UDP_Conn;
          else
            obj.COM_Conn = [];
            clear obj.COM_Conn;
          end
      end
      
      function sendRaw(obj, text)
          try
              if obj.Method == 0
                  pnet(obj.UDP_Conn,'printf', strcat(text, newline));
                  pnet(obj.UDP_Conn,'writepacket', obj.UDP_IP, obj.UDP_Port);
              else 
                  write(obj.COM_Conn, text, 'string');
              end
          catch e
              fprintf(1,'Error message:\n%s',e.message);
              warning('Could not send command to PupilEXT host computer.');
          end
      end
      
      function setImageRecordingPath(obj, imageRecordingPath)
          obj.sendRaw(['GP ' imageRecordingPath]);
      end
      
      function setDataRecordingPathAndName(obj, dataRecordingPathAndName)
          obj.sendRaw(['GL ' dataRecordingPathAndName]);
      end
      
      function setDataRecordingDelimiter(obj)
          obj.sendRaw(['GC ' obj.DataRecordingDelimiter]);
      end
      
      function setImageRecordingFormat(obj)
          obj.sendRaw(['GI ' obj.ImageRecordingFormat]);
      end
      
   end
   
   %%
   
   methods (Access = public)
       
      function obj = PupilEXT()
          obj.printVersion();
          disp('PupilEXT Remote Control Connection instance created.');
      end
      
      function obj = setupHostConnection(obj)
          if ~obj.Enabled, return, end
          obj = setupConnectionLayer(obj);
          pause(0.5);
      end
      
      function obj = closeHostConnection(obj)
          if ~obj.Enabled, return, end
          obj.stopDataRecording();
          obj.stopImageRecording();
          obj = obj.freeConnectionLayer();
          disp('Closed connection to PupilEXT host computer.');
      end
      
      function incrementTrial(obj)
          if ~obj.Enabled, return, end
          obj.sendRaw('T');
      end
      
      function sendMessage(obj, text)
          if ~obj.Enabled, return, end
          obj.sendRaw(['M ' text]);
      end
      
      function openSingleCamera(obj, cameraName)
          if ~obj.Enabled, return, end
          obj.sendRaw(['A1 ' cameraName]);
      end
      
      function openStereoCamera(obj, camera1Name, camera2Name)
          if ~obj.Enabled, return, end
          obj.sendRaw(['A2 ' camera1Name ';' camera2Name]);
      end
      
      function openUVCCamera(obj, deviceID)
          if ~obj.Enabled, return, end
          if isnumeric(deviceID)
            obj.sendRaw(['AW ' num2str(deviceID)]);
          else
            obj.sendRaw(['AW ' deviceID]);
          end
      end
      
      function startTracking(obj)
          if ~obj.Enabled, return, end
          obj.sendRaw('AT');
      end
      
      function stopTracking(obj)
          if ~obj.Enabled, return, end
          obj.sendRaw('AX');
      end
      
      function startDataRecording(obj)
          if ~obj.Enabled, return, end
          obj.setDataRecordingDelimiter();
          unixTimeNowMs = num2str(floor(posixtime(datetime('now','TimeZone','local'))*1000));
          obj.setDataRecordingPathAndName([obj.RecordingsPath '/' obj.ParticipantName '_' num2str(unixTimeNowMs) '.csv']);
          pause(0.5);
          obj.sendRaw('AR');
      end
      
      function stopDataRecording(obj)
          if ~obj.Enabled, return, end
          obj.sendRaw('AS');
      end
      
      function startDataStreaming(obj)
          if ~obj.Enabled, return, end
          obj.sendRaw('AV');
      end
      
      function stopDataStreaming(obj)
          if ~obj.Enabled, return, end
          obj.sendRaw('AC');
      end
      
      function startImageRecording(obj)
          if ~obj.Enabled, return, end
          obj.setImageRecordingFormat();
          unixTimeNowMs = num2str(floor(posixtime(datetime('now','TimeZone','local'))*1000));
          obj.setImageRecordingPath([obj.RecordingsPath '/' obj.ParticipantName '_' num2str(unixTimeNowMs)]);
          pause(0.5);
          obj.sendRaw('AM');
      end
      
      function stopImageRecording(obj)
          if ~obj.Enabled, return, end
          obj.sendRaw('AA');
      end
      
      function disconnectCamera(obj)
          if ~obj.Enabled, return, end
          obj.sendRaw('AD');
      end
      
      function forceResetTrialCounter(obj)
          if ~obj.Enabled, return, end
          obj.sendRaw('AF');
      end
      
      function setPupilDetectionAlgorithm(obj, algorithmName)
          if ~obj.Enabled, return, end
          obj.sendRaw(['PA ' algorithmName]);
      end
      
      function setUsingROIAreaSelection(obj, state)
          if ~obj.Enabled, return, end
          if state
            obj.sendRaw('PR true');
          else
            obj.sendRaw('PR false');
          end
      end
      
      function setComputeOutlineConfidence(obj, state)
          if ~obj.Enabled, return, end
          if state
            obj.sendRaw('PO true');
          else
            obj.sendRaw('PO false');
          end
      end
      
   end
   
end