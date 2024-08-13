
# This is just a test script

from PupilEXT import PupilEXT


pupilEXT = PupilEXT()
pupilEXT.Enabled = True

pupilEXT.Method = 0
pupilEXT.UDP_IP = '127.0.0.1'
pupilEXT.UDP_Port = 6900
# For simplicity we use the same local port number as the target port 
# number by default, but if you operate on localhost (i.e. the experiment 
# computer and host computer are physically the same), then you may
# want to specify some other local port because Matlab and PupilEXT cannot
# bind the same one port for themselves.
pupilEXT.UDP_LocalPort = 6500

# pupilEXT.Method = 1
# pupilEXT.COM_Port = 'COM6'
# pupilEXT.COM_BaudRate = 115200

pupilEXT.RecordingsPath = 'C:/PupilEXT_Recordings'
pupilEXT.ParticipantName = '1234'
pupilEXT.DataRecordingDelimiter = ';'
pupilEXT.ImageRecordingFormat = 'tiff'


pupilEXT.setupHostConnection()

pupilEXT.incrementTrial()
pupilEXT.sendMessage('TRIAL ' + str(1))

pupilEXT.openSingleCamera('Basler camera name')
pupilEXT.openStereoCamera('Basler camera name 1', 'Basler camera name 2')
pupilEXT.openUVCCamera(0)
pupilEXT.startTracking()
pupilEXT.stopTracking()
pupilEXT.startDataRecording()
pupilEXT.stopDataRecording()
pupilEXT.startDataStreaming()
pupilEXT.stopDataStreaming()
pupilEXT.startImageRecording()
pupilEXT.stopImageRecording()
pupilEXT.disconnectCamera()
pupilEXT.forceResetTrialCounter()
pupilEXT.setPupilDetectionAlgorithm('ElSe')
pupilEXT.setUsingROIAreaSelection(True)
pupilEXT.setComputeOutlineConfidence(True)
pupilEXT.connectRemoteControlUDP('192.168.40.3', 6900)
pupilEXT.connectRemoteControlCOM('COM1', 115200)
pupilEXT.disconnectRemoteControlUDP()
pupilEXT.disconnectRemoteControlCOM()
pupilEXT.connectStreamingUDP('192.168.40.3', 6900)
pupilEXT.connectStreamingCOM('COM1', 115200)
pupilEXT.disconnectStreamingUDP()
pupilEXT.disconnectStreamingCOM()
pupilEXT.connectMicrocontrollerUDP('192.168.40.200', 7000)
pupilEXT.connectMicrocontrollerCOM('COM1', 115200)
pupilEXT.disconnectMicrocontroller()
pupilEXT.switchToHardwareTriggeringMode()
pupilEXT.switchToSoftwareTriggeringMode()
pupilEXT.startHardwareTriggering()
pupilEXT.stopHardwareTriggering()
pupilEXT.setHardwareTriggeringLineSource(1)
pupilEXT.setHardwareTriggeringRuntimeLength(0)
pupilEXT.setHardwareTriggeringFramerate(50)
pupilEXT.setSoftwareTriggeringFramerateLimitingEnabled(True)
pupilEXT.setSoftwareTriggeringFramerateLimit(50)
pupilEXT.setExposureTimeMicrosec(4000)
pupilEXT.setGain(1.2)

pupilEXT.closeHostConnection()
