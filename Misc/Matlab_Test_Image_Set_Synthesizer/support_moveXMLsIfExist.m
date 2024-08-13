function support_moveXMLsIfExist(fromFolder, toFolder)
    dfne = dir(char([fromFolder '/*.xml']));
    if isempty(dfne)
        return;
    end
    for i = 1:length(dfne)
        movefile([fromFolder '/' dfne(i).name], [toFolder '/' dfne(i).name]);
    end
end