% Sample experiment code for PupilEXT integration (currently v0.1.2 beta)
% These code use Psychtoolbox 3
% Author: Gabor Benyei
% Note: This code was tested to work with Matlab R2020b

%% Config

participantName = '1234';
    
numTrials = 10;
    
timeFixOnScreen = 3.250;
timeStimOnScreen = 4.000;
timeBlinksAllowed = 3.000;


pupilEXT = PupilEXT();
pupilEXT.Enabled = true; % Easy way to turn off if you are debugging

pupilEXT.Method = 0;
pupilEXT.UDP_IP = '192.168.40.1';
pupilEXT.UDP_Port = 6900;

% pupilEXT.Method = 1;
% pupilEXT.COM_Port = 'COM1';
% pupilEXT.COM_BaudRate = 9600;

pupilEXT.RecordingsPath = 'C:/PupilEXT_Recordings'; % A slash at the end is not necessary
pupilEXT.ParticipantName = participantName;
% pupilEXT.DataRecordingDelimiter = ';';
pupilEXT.ImageRecordingFormat = 'tiff';
    
    
%% Init

whichScreen =  max(Screen('Screens'));
% whichScreen = 0;
    
grayColor=[60, 60, 60]; % very dark gray
imgBright = imread('bright.png');
    
Screen('Preference', 'SkipSyncTests', 1);
    

%% Setup PupilEXT
pupilEXT = pupilEXT.setupHostConnection();
    

%% Opening the window
window = Screen(whichScreen,'OpenWindow', grayColor);
Priority(MaxPriority(window));
HideCursor;
    
texBright = Screen('MakeTexture', window, imgBright);
    
[wWidth,wHeight]=Screen('WindowSize', window); 
Screen('TextSize', window, floor(wHeight/35));  
Screen('TextFont', window,'Microsoft Sans Serif'); % vagy Helvetica
Screen('TextStyle', window, 0); 


%% Presenting instructions
Screen('TextSize', window, floor(wHeight/35));  
beginY = 280;
spaceY = 60;
DrawFormattedText(window, 'Welcome!', 'center', beginY );
DrawFormattedText(window, 'In this experiment your task is to fixate on the + sign in the middle of the screen.', 'center', beginY +spaceY );
DrawFormattedText(window, 'The background will periodically change, but you do not need to respond to that.', 'center', beginY +spaceY*2 );
DrawFormattedText(window, 'Please look at the screen as long as the + sign is visible, and blink only when you see the letter B.', 'center', beginY +spaceY*3 );
DrawFormattedText(window, 'The experiment will take just a few minutes.', 'center', beginY +spaceY*4 );
DrawFormattedText(window, '[If you feel ready, hit SPACE to begin]', 'center', beginY +spaceY*7 );
Screen(window,'Flip');

KbWait_c_esc(KbName('space'));

% pupilEXT.startDataRecording();
pupilEXT.startImageRecording();
    
% To surely register a few seconds beginning of data
Screen(window,'Flip');
WaitSecs(2); 

Screen('TextSize',window, floor(wHeight/20));
DrawFormattedText(window, 'B', 'center', 'center', 0);
Screen(window,'Flip');
WaitSecs(timeBlinksAllowed);


%% Presenting stimuli
for j = 1 : numTrials

    Screen('TextSize',window, floor(wHeight/20));
    DrawFormattedText(window, '+', 'center', 'center', 0);
    
    Screen(window,'Flip');
    WaitSecs(timeFixOnScreen);
    
        
    bgRect=[0 0 wWidth wHeight];
    Screen('DrawTexture', window, texBright, [], bgRect );
    Screen('TextSize',window, floor(wHeight/20));
    DrawFormattedText(window, '+', 'center', 'center', 0);
    
    pupilEXT.incrementTrial();
    % NOTE: You can use trial increments or messages too. The advantage of 
    % trial increments is that the output .csv data of eye samples will 
    % contain a separate column for trial numbers of each sample, making 
    % it easier to process by custom pipelines. The messages will be 
    % recorded on the other side as precisely as trial increments, 
    % regarding their timestamps. To best account for any additional 
    % unwanted lag on the experiment PC side (type conversions, 
    % serialization), the easiest is to add the frame buffer flip 
    % instruction of Psychtoolbox (Screen(window,'Flip');) right after the 
    % pupilEXT.incrementTrial(); or pupilEXT.sendMessage(); call(s), as the
    % program is executed serially in Matlab.
    pupilEXT.sendMessage(['TRIAL ' num2str(j)]);
    
    Screen(window,'Flip');
    WaitSecs(timeStimOnScreen);
    
        
    Screen('TextSize',window, floor(wHeight/20));
    DrawFormattedText(window, 'B', 'center', 'center', 0);
    Screen(window,'Flip');
    WaitSecs(timeBlinksAllowed);

end

Screen('TextSize',window, floor(wHeight/20));
DrawFormattedText(window, '+', 'center', 'center', 0);
Screen(window,'Flip');
    
% To surely register end of ongoing neural/eye response
WaitSecs(4); 
    
% pupilEXT.stopDataRecording();
pupilEXT.stopImageRecording();

% ... Other blocks can come here
    
%% Closing 
Screen(window,'Flip');
    
pupilEXT = pupilEXT.closeHostConnection();
    
% ... Save behav data here if needed
    
Screen('TextSize',window, floor(wHeight/30)); 
DrawFormattedText(window, 'The experiment has ended. Thank you for your participation!', 'center', 'center');
Screen(window,'Flip');
    
KbWait_c(KbName('space'));
    
Screen('CloseAll');



