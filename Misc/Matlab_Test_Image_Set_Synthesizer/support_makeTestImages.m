function support_makeTestImages(Config)

    if ~isfield(Config, 'outFolder') || isempty(Config.outFolder)
        Config.outFolder = ['single_testimages_' num2str(Config.minPupDilMm) '_' num2str(Config.maxPupDilMm) '_' num2str(Config.fps) '_' num2str(Config.numTrials) '_' Config.type '_' num2str(Config.phaseShiftRad)];
    end

    from = 0;
    by = 1000/Config.fps;
    to = (Config.oneTrialLenMs/1000*Config.fps*Config.numTrials -1) *by;
    t = from : by : to;
    
    disp(num2str(t(2),'%12.f'));

    vecIn = 2*pi *(t./to) *Config.numTrials;
    vecIn = vecIn + Config.phaseShiftRad;
    if strcmp(Config.type, 'square')
        y = square(vecIn);
    elseif strcmp(Config.type, 'sawtooth')
        y = sawtooth(vecIn);
    elseif strcmp(Config.type, 'triangle')
        y = sawtooth(vecIn, 0.5);
    elseif strcmp(Config.type, 'sin')
        y = sin(vecIn);
    else
        return;
    end
    y = (y+1)./2;
    y = y.*(Config.maxPupDilMm-Config.minPupDilMm);
    y = y+Config.minPupDilMm;
    
    t = t + Config.startTimeMs; % has to be here (later than making vecIn from t)

%     plot(t,y)
%     grid on

    support_wipeFolderIfExists(Config.outFolder);
    support_makeFolderIfDoesntExist(Config.outFolder);
    
    for i = 1:length(y)

        imgTimestamp = t(i);
        fullTimeStampStr = num2str(imgTimestamp,'%12.f');
        
        pupRadiusPx = round([y(i)/2] * Config.resolutionDPMM);

        img = ones(Config.imageSizeYPx, Config.imageSizeXPx);
        img = uint8(255 * img); % binary to greyscale
        
        img = support_drawCircleInImage(img, Config.imageSizeXPx/2, (Config.imageSizeYPx*5/6)/2, pupRadiusPx, 0);
        
%         img = support_fillCheckerPatternInImage(img, 0, 1, 5/6, 1, Config.resolutionDPMM, Config.checkerSizeMm, 235, 165);
        img = support_fillCheckerPatternInImage(img, 0, Config.safeAreaSizeYMm, Config.imageSizeXMm/Config.fillCheckerSizeMm, (Config.imageSizeYMm-Config.safeAreaSizeYMm)/Config.fillCheckerSizeMm, Config.resolutionDPMM, Config.fillCheckerSizeMm, 235, 165);

        img = support_putTextInImage(img, ['Timestamp = ' fullTimeStampStr ' [ms]'], 0.08, 0.85, 235);
        img = support_putTextInImage(img, ['Checker size = ' num2str(Config.fillCheckerSizeMm) ' [mm] @ ' num2str(Config.resolutionDPMM) ' DPMM'], 0.52, 0.85, 235);
        img = support_putTextInImage(img, ['Pupil size = ' num2str(round(pupRadiusPx*2)) ' [px] = ' num2str(round(pupRadiusPx*2 *1/Config.resolutionDPMM,2)) ' [mm]'], 0.08, 0.92, 235);
            
%         imshow(img, []);
%         truesize(); 
        
        % Saving
        fullFileName = [Config.outFolder '/' fullTimeStampStr '.png'];
        disp(['Writing image file: ' fullFileName]);
        
        imwrite(img, fullFileName, 'png');

    end
    disp(['Success. Number of images generated: ' num2str(length(t))]);
    
    if Config.makeEventLog
        disp(['Making event log...']);
        eventLogContents = cell(Config.numTrials, 2);
        fid = fopen([Config.outFolder '/offline_event_log.xml'],'w');
        line = ['<RecordedEvents>'];
        fprintf( fid, '%s\n', line );
        for c = 1:Config.numTrials
            tIndex = Config.oneTrialLenMs/1000*Config.fps*(c-1) +1;
            tst = num2str(t(tIndex),'%12.f');
            trn = num2str(c);
            line = ['<TrialIncrement TrialNumber="' trn '" TimestampMs="' tst '" />'];
            fprintf( fid, '%s\n', line );
        end
        for c = 1:Config.numTrials
            tIndex = Config.oneTrialLenMs/1000*Config.fps*(c-1) +1;
            tst = num2str(t(tIndex),'%12.f');
            tmg = ['STIM_PRES_' num2str(c)];
            line = ['<Message MessageString="' tmg '" TimestampMs="' tst '" />'];
            fprintf( fid, '%s\n', line );
            %
            tst = num2str(t(tIndex+(Config.oneTrialLenMs/2)/1000*Config.fps),'%12.f');
            tmg = ['RESP_' num2str(c)];
            line = ['<Message MessageString="' tmg '" TimestampMs="' tst '" />'];
            fprintf( fid, '%s\n', line );
        end
        line = ['</RecordedEvents>'];
        fprintf( fid, '%s\n', line );
        fclose(fid);
        disp(['Success.']);
    end
    
end