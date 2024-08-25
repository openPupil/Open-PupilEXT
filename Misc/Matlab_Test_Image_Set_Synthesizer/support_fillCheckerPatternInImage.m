function img = support_fillCheckerPatternInImage(img, fromXMm, fromYMm, timesX, timesY, resolutionDPMM, checkerSizeMm, color1Byte, color2Byte)
    
    fromX = floor(fromXMm * resolutionDPMM)+1;
    fromY = floor(fromYMm * resolutionDPMM)+1;
    csp = floor(checkerSizeMm * resolutionDPMM);
    toX = fromX-1 +csp*timesX;
    toY = fromY-1 +csp*timesY;
    
    mask = false(size(img));
    thisOrThat = true;
    for row = fromY:toY
        for col = fromX:toX
            mask(row, col) = thisOrThat;
            if col-fromX > 0 && (mod(col-fromX,csp)==0 || (col==toX && mod(timesX,2)==0))
%                 disp(['row = ' num2str(row) ' ; col = ' num2str(col)]);
                thisOrThat = ~thisOrThat;
            end
        end
        if row-fromY > 0 && mod(row-fromY,csp)==0
            thisOrThat = ~thisOrThat;
        end
    end
    
    img2 = ones(size(img));
    img2 = uint8(255 * img2);
    
    locMask = false(size(img));
    locMask(fromY:toY, fromX:toX) = true;
    img2(mask & locMask) = color1Byte;
    img2(~mask & locMask) = color2Byte;
    
    % combine them in grayscale
    img(img2 < 255) = img2(img2 < 255);
    
end