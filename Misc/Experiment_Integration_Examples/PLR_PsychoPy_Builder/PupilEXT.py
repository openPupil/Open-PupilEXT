# PupilEXT Remote Control Connection helper class for PsychoPy
# Compatible with the PupilEXT software (currently v0.1.2 beta)
# Author: Gabor Benyei
# License: GNU GPL v3 (identical to that of the PupilEXT software, see "Misc/LICENSE")
# Note: This code was tested to work with PsychoPy 2023-2-3 but it should work with way earlier versions too

import re
import socket
import serial
import time
import numbers

class PupilEXT(object):
    
    # NOTE: These are just internal defaults, you do not need to change them
    Version = 1
    Enabled = False
    Method = 0 
    UDP_IP = '192.168.40.1'
    UDP_Port = 6900
    COM_Port = 'COM1'
    COM_BaudRate = 9600
    # COM_ByteOrder = 
    COM_FlowControl = False
    COM_StopBits = serial.STOPBITS_ONE
    COM_DataBits = serial.EIGHTBITS
    COM_Parity = serial.PARITY_NONE
    COM_Timeout = 10
    COM_Terminator = '\n'
    RecordingsPath = 'C:/PupilEXT_Recordings/'
    ParticipantName = '1234'
    DataRecordingDelimiter = ';'
    ImageRecordingFormat = 'tiff'
    
    ##
    
    def __setupConnectionLayer(self):
        try:
            if self.Method == 0:
                self.UDP_Conn = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)
            else:
                self.COM_Conn = serial.Serial(
                    port = self.COM_Port, 
                    baudrate = self.COM_BaudRate, 
                    bytesize = self.COM_DataBits, 
                    parity = self.COM_Parity, 
                    stopbits = self.COM_StopBits, 
                    timeout = self.COM_Timeout, 
                    xonxoff = self.COM_FlowControl, 
                    write_timeout = self.COM_Timeout, 
                    inter_byte_timeout = self.COM_Timeout)
        except Exception as e:
            print('Error message:\n' + str(e))
            print('Could not setup connection to PupilEXT host computer.')
      
    def __freeConnectionLayer(self):
        if self.Method == 0:
            self.UDP_Conn.close()
        else:
            self.COM_Conn.close()

    def __sendRaw(self, text):
        try:
            if self.Method == 0:
                self.UDP_Conn.sendto((text + '\n').encode('iso-8859-1'), (self.UDP_IP, self.UDP_Port))
            else: 
                self.COM_Conn.write((text + self.COM_Terminator).encode('iso-8859-1'))
        except Exception as e:
            print('Error message:\n' + str(e))
            print('Could not send command to PupilEXT host computer.')
      
    def __setImageRecordingPath(self, imageRecordingPath):
        self.__sendRaw('GP ' + imageRecordingPath)
      
    def __setDataRecordingPathAndName(self, dataRecordingPathAndName):
        self.__sendRaw('GL ' + dataRecordingPathAndName)
      
    def __setDataRecordingDelimiter(self):
        self.__sendRaw('GC ' + self.DataRecordingDelimiter)
      
    def __setImageRecordingFormat(self):
        self.__sendRaw('GI ' + self.ImageRecordingFormat)
        
    def __printVersion(self):
        print('Using PupilEXT Remote Control Connection helper class VERSION ' + str(self.Version) + ' for PsychoPy.')
    
    ##
    
    def __init__(self):
        self.__printVersion()
        print('PupilEXT Remote Control Connection instance created.')
    
    def setupHostConnection(self):
        if not self.Enabled : return
        self.__setupConnectionLayer()
        time.sleep(0.5)
      
    def closeHostConnection(self):
        if not self.Enabled : return
        self.stopDataRecording()
        self.stopImageRecording()
        self.__freeConnectionLayer()
        print('Closed connection to PupilEXT host computer.')
      
    def incrementTrial(self):
        if not self.Enabled : return
        self.__sendRaw('T')
      
    def sendMessage(self, text):
        if not self.Enabled : return
        self.__sendRaw('M ' + text)
      
    def openSingleCamera(self, cameraName):
        if not self.Enabled : return
        self.__sendRaw('A1 ' + cameraName)
      
    def openStereoCamera(self, camera1Name, camera2Name):
        if not self.Enabled : return
        self.__sendRaw('A2 ' + camera1Name + ';' + camera2Name)
      
    def openUVCCamera(self, deviceID):
        if not self.Enabled : return
        if not isinstance(deviceID, str):
            self.__sendRaw('AW ' + str(deviceID))
        else:
            self.__sendRaw('AW ' + deviceID)
      
    def startTracking(self):
        if not self.Enabled : return
        self.__sendRaw('AT')
      
    def stopTracking(self):
        if not self.Enabled : return
        self.__sendRaw('AX')
      
    def startDataRecording(self):
        if not self.Enabled : return
        self.__setDataRecordingDelimiter()
        unixTimeNowMs = int(time.time())
        self.__setDataRecordingPathAndName(self.RecordingsPath + '/' + self.ParticipantName + '_' + str(unixTimeNowMs) + '.csv');
        time.sleep(0.5)
        self.__sendRaw('AR')
      
    def stopDataRecording(self):
        if not self.Enabled : return
        self.__sendRaw('AS')
      
    def startDataStreaming(self):
        if not self.Enabled : return
        self.__sendRaw('AV')
      
    def stopDataStreaming(self):
        if not self.Enabled : return
        self.__sendRaw('AC')
      
    def startImageRecording(self):
        if not self.Enabled : return
        self.__setImageRecordingFormat()
        unixTimeNowMs = int(time.time())
        self.__setImageRecordingPath(self.RecordingsPath + '/' + self.ParticipantName + '_' + str(unixTimeNowMs))
        time.sleep(0.5)
        self.__sendRaw('AM')
      
    def stopImageRecording(self):
        if not self.Enabled : return
        self.__sendRaw('AA')
      
    def disconnectCamera(self):
        if not self.Enabled : return
        self.__sendRaw('AD')
      
    def forceResetTrialCounter(self):
        if not self.Enabled : return
        self.__sendRaw('AF')
      
    def setPupilDetectionAlgorithm(self, algorithmName):
        if not self.Enabled : return
        self.__sendRaw('PA ' + algorithmName)
      
    def setUsingROIAreaSelection(self, state):
        if not self.Enabled : return
        if state:
            self.__sendRaw('PR true')
        else:
            self.__sendRaw('PR false')
      
    def setComputeOutlineConfidence(self, state):
        if not self.Enabled : return
        if state:
            self.__sendRaw('PO true')
        else:
            self.__sendRaw('PO false')
    
