function support_makeCalibImages(Config)

    if ~isfield(Config, 'outFolder') || isempty(Config.outFolder)
        Config.outFolder = ['single_testimages'];
    end
    
    support_wipeFolderIfExists(Config.outFolder);
    support_makeFolderIfDoesntExist(Config.outFolder);

    for i = 1:Config.numCalibImages

        img = ones(Config.imageSizeYPx, Config.imageSizeXPx);
        img = uint8(255 * img); % binary to greyscale

        borderSquares = 1;
        checkerFitsNX = floor((Config.imageSizeXMm/Config.calibCheckerSizeMm) - 2*borderSquares);
        checkerFitsNY = floor((Config.safeAreaSizeYMm/Config.calibCheckerSizeMm)- 2*borderSquares);
        img = support_fillCheckerPatternInImage(img, ...
            borderSquares *Config.calibCheckerSizeMm, ...
            borderSquares *Config.calibCheckerSizeMm, ...
            checkerFitsNX, ...
            checkerFitsNY, ...
            Config.resolutionDPMM, Config.calibCheckerSizeMm, 0, 255);

        img(Config.safeAreaSizeYMm*Config.resolutionDPMM : (Config.safeAreaSizeYMm*Config.resolutionDPMM+1), :) = 0;

        img = support_putTextInImage(img, ['Image nr.: ' num2str(i) ' / ' num2str(Config.numCalibImages) ], 0.08, 0.85, 235);
        img = support_putTextInImage(img, ['Checker size = ' num2str(Config.calibCheckerSizeMm) ' [mm]'], 0.42, 0.85, 235);
        img = support_putTextInImage(img, ['Checkerboard size (WxH) = ' num2str(checkerFitsNX) ' x ' num2str(checkerFitsNY) ' = ' num2str(checkerFitsNX*Config.calibCheckerSizeMm) ' x ' num2str(checkerFitsNY*Config.calibCheckerSizeMm) ' [mm]'], 0.08, 0.92, 235);

    %     imshow(img, []);
    %     truesize(); 

        % Saving
        imgTimestamp = ((i-1)*1/Config.fps*1000); 
        fullTimeStampStr = num2str(imgTimestamp,'%12.f');
        fullFileName = [Config.outFolder '/' fullTimeStampStr '.png'];
        disp(['Writing image file: ' fullFileName]);

        imwrite(img, fullFileName, 'png');

    end

end