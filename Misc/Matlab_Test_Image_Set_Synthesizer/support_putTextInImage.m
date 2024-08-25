function img = support_putTextInImage(img, text, xPosNorm, yPosNorm, boxColorByte)
    img2 = ones(size(img,1), size(img,2));
    img2 = uint8(255 * img2);
    img2 = rgb2gray(insertText(img2, [size(img,2)*xPosNorm, size(img,1)*yPosNorm], text, 'Font', 'ISOCPEUR', 'FontSize', round(size(img,1)/30), 'BoxColor', [boxColorByte boxColorByte boxColorByte]));
    
    % combine them in grayscale
    img(img2 < 255) = img2(img2 < 255);
end