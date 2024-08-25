
SET_DEFAULTS

Config.numTrials = 3;
Config.fps = 50;

Config.minPupDilMm = 3;
Config.maxPupDilMm = 8;

Config.phaseShiftRad = 0;

Config.type = 'triangle';
Config.outFolder = ['1cam1pup_single_pupil_images_' num2str(Config.minPupDilMm) '_' num2str(Config.maxPupDilMm) '_' num2str(Config.fps) '_' num2str(Config.numTrials)];
support_makeTestImages(Config);

tempFolder1 = 'temp1';
tempFolder2 = 'temp2';
Config.type = 'triangle';
Config.outFolder = tempFolder1;
support_makeTestImages(Config);
Config.type = 'sin';
Config.outFolder = tempFolder2;
support_makeTestImages(Config);
outFolder = ['1cam2pup_single_pupil_images_' num2str(Config.minPupDilMm) '_' num2str(Config.maxPupDilMm) '_' num2str(Config.fps) '_' num2str(Config.numTrials)];
support_joinImages(tempFolder1, tempFolder2, outFolder);
support_moveXMLsIfExist(tempFolder1, outFolder);
rmdir(tempFolder1, 's');
rmdir(tempFolder2, 's');

folderBase = ['2cam1pup_stereo_pupil_images_' num2str(Config.minPupDilMm) '_' num2str(Config.maxPupDilMm) '_' num2str(Config.fps) '_' num2str(Config.numTrials)];
Config.type = 'triangle';
Config.makeEventLog = true;
Config.outFolder = [folderBase '/0'];
support_makeTestImages(Config);
support_moveXMLsIfExist(Config.outFolder, folderBase);
Config.type = 'sin';
Config.makeEventLog = false;
Config.outFolder = [folderBase '/1'];
support_makeTestImages(Config);
Config.makeEventLog = true;

folderBase = ['2cam2pup_stereo_pupil_images_' num2str(Config.minPupDilMm) '_' num2str(Config.maxPupDilMm) '_' num2str(Config.fps) '_' num2str(Config.numTrials)];
tempFolder1 = 'temp1';
tempFolder2 = 'temp2';
Config.type = 'triangle';
Config.outFolder = tempFolder1;
support_makeTestImages(Config);
Config.type = 'sin';
Config.outFolder = tempFolder2;
support_makeTestImages(Config);
outFolder = [folderBase '/0'];
support_joinImages(tempFolder1, tempFolder2, outFolder);
% support_copyXMLsIfExist(tempFolder1, outFolder);
rmdir(tempFolder1, 's');
rmdir(tempFolder2, 's');

Config.type = 'sawtooth';
Config.outFolder = tempFolder1;
support_makeTestImages(Config);
Config.type = 'square';
Config.outFolder = tempFolder2;
support_makeTestImages(Config);
outFolder = [folderBase '/1'];
support_joinImages(tempFolder1, tempFolder2, outFolder);
support_moveXMLsIfExist(tempFolder1, folderBase);
rmdir(tempFolder1, 's');
rmdir(tempFolder2, 's');

% Config.outFolder = ['single_calib_images'];
% support_makeCalibImages(Config);
% 
% folderBase = ['stereo_calib_images'];
% Config.outFolder = [folderBase '/0'];
% support_makeCalibImages(Config);
% Config.outFolder = [folderBase '/1'];
% support_makeCalibImages(Config);