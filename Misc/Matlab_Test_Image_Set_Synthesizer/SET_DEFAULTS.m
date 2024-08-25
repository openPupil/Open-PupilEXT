
close all
clear

Config.resolutionDPMM = 20; % any integer, best whole divisible by 10, greater or equal to *******************
Config.fillCheckerSizeMm = 1;
Config.calibCheckerSizeMm = 4; % any integer greater or equal to 1

Config.numCalibImages = 200;

Config.imageSizeXMm = 36; % any integer whole divisible by 12, greater or equal to 36
Config.safeAreaSizeYMm = Config.imageSizeXMm/4*3; % automatically set for 4:3 aspect ratio
Config.imageSizeYMm = Config.safeAreaSizeYMm *6/5; % automatically set for embossed text

Config.imageSizeXPx = floor(Config.imageSizeXMm * Config.resolutionDPMM);
Config.imageSizeYPx = floor(Config.imageSizeYMm * Config.resolutionDPMM);

Config.minPupDilMm = 2;
Config.maxPupDilMm = 8;

Config.numTrials = 3;
Config.startTimeMs = 1000000000000;
Config.oneTrialLenMs = 3000;
Config.fps = 100;

% Config.phaseShiftRad = 1*pi;
Config.phaseShiftRad = 0;

% Config.type = 'square';
% Config.type = 'sawtooth';
Config.type = 'triangle';
% Config.type = 'sin';

% Config.outFolder = 'output';

Config.numPrefix = 1000;

% Config.makeEventLog = false;
Config.makeEventLog = true;
