function img = support_drawCircleInImage(img, centerXpx, centerYpx, radiusPx, colorByte)
    img2 = ones(size(img,2), size(img,1));
    [x, y] = meshgrid(1:size(img,2), 1:size(img,1));
    img2((x - centerXpx).^2 + (y - centerYpx).^2 <= radiusPx.^2) = colorByte;
    img2 = uint8(255 * img2);
    
    % combine them in grayscale
    img(img2 < 255) = img2(img2 < 255);
end