function support_joinImages(inFolder1, inFolder2, outFolder)
    dfn1 = dir(char([inFolder1 '/*.png']));
    dfn2 = dir(char([inFolder1 '/*.png']));
    
    for i = 1:length(dfn1)
        if dfn1(i).name ~= dfn2(i).name
            error('Contents of the two image directories do not match');
        end
    end
    
    dfn = dfn1;
    clear dfn1 dfn2;
    
    support_wipeFolderIfExists(outFolder);
    support_makeFolderIfDoesntExist(outFolder);
    
    for i = 1:length(dfn)
        disp(['Currently processing: ' dfn(i).name]);
        imgIn1 = imread([inFolder1 '/' dfn(i).name]);
        imgIn2 = imread([inFolder2 '/' dfn(i).name]);
        imgOut = [imgIn1 imgIn2];
        imgOut(:,(size(imgOut,2)/2):(size(imgOut,2)/2+1)) = 0;
        imwrite(imgOut, [outFolder '/' dfn(i).name], 'png');
    end
end