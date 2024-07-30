% This is just a test script

clear
clc


pupilEXT = PupilEXT();
pupilEXT.Enabled = true;

% pupilEXT.Method = 0;
% pupilEXT.UDP_IP = '192.168.40.1';
% pupilEXT.UDP_Port = 6900;

pupilEXT.Method = 1;
pupilEXT.COM_Port = 'COM6';
pupilEXT.COM_BaudRate = 9600;

pupilEXT.RecordingsPath = 'C:/PupilEXT_Recordings';
pupilEXT.ParticipantName = '1234';
pupilEXT.DataRecordingDelimiter = ';';
pupilEXT.ImageRecordingFormat = 'tiff';


pupilEXT = pupilEXT.setupHostConnection();

pupilEXT.incrementTrial();
pupilEXT.sendMessage(['TRIAL ' num2str(1)]);

pupilEXT.openSingleCamera('Basler camera name')
pupilEXT.openStereoCamera('Basler camera name 1', 'Basler camera name 2')
pupilEXT.openUVCCamera(0);
pupilEXT.startTracking();
pupilEXT.stopTracking();
pupilEXT.startDataRecording();
pupilEXT.stopDataRecording();
pupilEXT.startDataStreaming();
pupilEXT.stopDataStreaming();
pupilEXT.startImageRecording();
pupilEXT.stopImageRecording();
pupilEXT.disconnectCamera();
pupilEXT.forceResetTrialCounter();
pupilEXT.setPupilDetectionAlgorithm('ElSe');
pupilEXT.setUsingROIAreaSelection(true);
pupilEXT.setComputeOutlineConfidence(true);

pupilEXT = pupilEXT.closeHostConnection();