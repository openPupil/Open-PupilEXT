function support_makeFolderIfDoesntExist(folder)
    if ~exist(folder, 'dir')
        disp('Did not find this output folder. Creating it...');
        mkdir(folder);
    end
end