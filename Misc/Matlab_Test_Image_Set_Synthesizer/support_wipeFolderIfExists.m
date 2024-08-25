function support_wipeFolderIfExists(folder) 
    if exist(folder, 'dir')
        disp('Found old files in the output folder. Deleting them...');
        rmdir(folder, 's');
        % Note: This can throw error if last time the code broke while a
        % file was open (image or event log). To solve, close and open
        % Matlab again. (Exists in at least R2020b.)
    end
end