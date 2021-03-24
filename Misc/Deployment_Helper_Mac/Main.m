%% Deployement Helper
% The VTK libraries are iterated to create commands for deployment.
% If this is not done, then PupilEXT cannot be run on a computer without the necessary C++ libraries


Filename = {'/Users/papillonmac/Desktop/PupilExt.app/Contents/Frameworks/libvtkCommonColor-9.0.1.dylib';
    '/Users/papillonmac/Desktop/PupilExt.app/Contents/Frameworks/libvtkCommonComputationalGeometry-9.0.1.dylib';
    '/Users/papillonmac/Desktop/PupilExt.app/Contents/Frameworks/libvtkCommonCore-9.0.1.dylib';
    '/Users/papillonmac/Desktop/PupilExt.app/Contents/Frameworks/libvtkCommonDataModel-9.0.1.dylib';
    '/Users/papillonmac/Desktop/PupilExt.app/Contents/Frameworks/libvtkCommonExecutionModel-9.0.1.dylib';
    '/Users/papillonmac/Desktop/PupilExt.app/Contents/Frameworks/libvtkCommonMath-9.0.1.dylib';
    '/Users/papillonmac/Desktop/PupilExt.app/Contents/Frameworks/libvtkCommonMisc-9.0.1.dylib';
    '/Users/papillonmac/Desktop/PupilExt.app/Contents/Frameworks/libvtkCommonSystem-9.0.1.dylib';
    '/Users/papillonmac/Desktop/PupilExt.app/Contents/Frameworks/libvtkCommonTransforms-9.0.1.dylib';
    '/Users/papillonmac/Desktop/PupilExt.app/Contents/Frameworks/libvtkDICOMParser-9.0.1.dylib';
    '/Users/papillonmac/Desktop/PupilExt.app/Contents/Frameworks/libvtkFiltersCore-9.0.1.dylib';
    '/Users/papillonmac/Desktop/PupilExt.app/Contents/Frameworks/libvtkFiltersExtraction-9.0.1.dylib';
    '/Users/papillonmac/Desktop/PupilExt.app/Contents/Frameworks/libvtkFiltersGeneral-9.0.1.dylib';
    '/Users/papillonmac/Desktop/PupilExt.app/Contents/Frameworks/libvtkFiltersGeometry-9.0.1.dylib';
    '/Users/papillonmac/Desktop/PupilExt.app/Contents/Frameworks/libvtkFiltersHybrid-9.0.1.dylib';
    '/Users/papillonmac/Desktop/PupilExt.app/Contents/Frameworks/libvtkFiltersModeling-9.0.1.dylib';
    '/Users/papillonmac/Desktop/PupilExt.app/Contents/Frameworks/libvtkFiltersSources-9.0.1.dylib';
    '/Users/papillonmac/Desktop/PupilExt.app/Contents/Frameworks/libvtkFiltersStatistics-9.0.1.dylib';
    '/Users/papillonmac/Desktop/PupilExt.app/Contents/Frameworks/libvtkFiltersTexture-9.0.1.dylib';
    '/Users/papillonmac/Desktop/PupilExt.app/Contents/Frameworks/libvtkfreetype-9.0.1.dylib';
    '/Users/papillonmac/Desktop/PupilExt.app/Contents/Frameworks/libvtkImagingCore-9.0.1.dylib';
    '/Users/papillonmac/Desktop/PupilExt.app/Contents/Frameworks/libvtkImagingFourier-9.0.1.dylib';
    '/Users/papillonmac/Desktop/PupilExt.app/Contents/Frameworks/libvtkImagingSources-9.0.1.dylib';
    '/Users/papillonmac/Desktop/PupilExt.app/Contents/Frameworks/libvtkInteractionStyle-9.0.1.dylib';
    '/Users/papillonmac/Desktop/PupilExt.app/Contents/Frameworks/libvtkIOCore-9.0.1.dylib';
    '/Users/papillonmac/Desktop/PupilExt.app/Contents/Frameworks/libvtkIOExport-9.0.1.dylib';
    '/Users/papillonmac/Desktop/PupilExt.app/Contents/Frameworks/libvtkIOGeometry-9.0.1.dylib';
    '/Users/papillonmac/Desktop/PupilExt.app/Contents/Frameworks/libvtkIOImage-9.0.1.dylib';
    '/Users/papillonmac/Desktop/PupilExt.app/Contents/Frameworks/libvtkIOLegacy-9.0.1.dylib';
    '/Users/papillonmac/Desktop/PupilExt.app/Contents/Frameworks/libvtkIOPLY-9.0.1.dylib';
    '/Users/papillonmac/Desktop/PupilExt.app/Contents/Frameworks/libvtkIOXML-9.0.1.dylib';
    '/Users/papillonmac/Desktop/PupilExt.app/Contents/Frameworks/libvtkIOXMLParser-9.0.1.dylib';
    '/Users/papillonmac/Desktop/PupilExt.app/Contents/Frameworks/libvtklibharu-9.0.1.dylib';
    '/Users/papillonmac/Desktop/PupilExt.app/Contents/Frameworks/libvtkloguru-9.0.1.dylib';
    '/Users/papillonmac/Desktop/PupilExt.app/Contents/Frameworks/libvtkmetaio-9.0.1.dylib';
    '/Users/papillonmac/Desktop/PupilExt.app/Contents/Frameworks/libvtkParallelCore-9.0.1.dylib';
    '/Users/papillonmac/Desktop/PupilExt.app/Contents/Frameworks/libvtkParallelDIY-9.0.1.dylib';
    '/Users/papillonmac/Desktop/PupilExt.app/Contents/Frameworks/libvtkRenderingContext2D-9.0.1.dylib';
    '/Users/papillonmac/Desktop/PupilExt.app/Contents/Frameworks/libvtkRenderingCore-9.0.1.dylib';
    '/Users/papillonmac/Desktop/PupilExt.app/Contents/Frameworks/libvtkRenderingFreeType-9.0.1.dylib';
    '/Users/papillonmac/Desktop/PupilExt.app/Contents/Frameworks/libvtkRenderingLOD-9.0.1.dylib';
    '/Users/papillonmac/Desktop/PupilExt.app/Contents/Frameworks/libvtkRenderingOpenGL2-9.0.1.dylib';
    '/Users/papillonmac/Desktop/PupilExt.app/Contents/Frameworks/libvtkRenderingSceneGraph-9.0.1.dylib';
    '/Users/papillonmac/Desktop/PupilExt.app/Contents/Frameworks/libvtkRenderingUI-9.0.1.dylib';
    '/Users/papillonmac/Desktop/PupilExt.app/Contents/Frameworks/libvtkRenderingVtkJS-9.0.1.dylib';
    '/Users/papillonmac/Desktop/PupilExt.app/Contents/Frameworks/libvtksys-9.0.1.dylib'};

Gesamtbefehl_Alles = {};
Gesamtbefehl = {};
for FileName_Index = 1:size(Filename, 1)
    
    [status,cmdout] = system(['otool -L ', Filename{FileName_Index,:}]);
    
    expression = '(/usr/local/Cellar/vtk/9.0.1_5/lib/)\w+';
    Startwerte = regexp(cmdout, expression);
    if isempty(Startwerte) ~=true
        Pfad_Bibliothek = {};
        Bibliothek_Name = {};
        for StartwertIndex = 1:size(Startwerte, 2)
            
            Aktueller_Startwert = Startwerte(StartwertIndex);
            Bool = 1;
            i = 1;
            
            while Bool == 1
                
                Aktueller_String = cmdout(Aktueller_Startwert+i:Aktueller_Startwert+i+5);
                i = i + 1;
                
                if strcmp(Aktueller_String, '.dylib')
                    
                    Bool = 0;
                    Pfad_Bibliothek{StartwertIndex,1} = cmdout(Aktueller_Startwert:Aktueller_Startwert+i+4);
                    
                    Cell_Array_SPLIT = split(Pfad_Bibliothek,"/");
                    Bibliothek_Name{StartwertIndex,1} = Cell_Array_SPLIT{end};
                    
                end
                
            end
            
        end
        
        % Construct the Strings
        % install_name_tool -change /usr/local/Cellar/vtk/9.0.1_5/lib/libvtkCommonTransforms-9.0.1.dylib
        % @executable_path/../Frameworks/libvtkCommonTransforms-9.0.1.dylib
        % /Users/papillonmac/Desktop/PupilExt.app/Contents/Frameworks/libvtkRenderingFreeType-9.0.1.dylib
        
        Gesamtbefehl = {};
        for Index = 1:size(Pfad_Bibliothek, 1)
            Part_1 = ['install_name_tool -change ', Pfad_Bibliothek{Index, 1}];
            
            Part_2 = ['@executable_path/../Frameworks/', Bibliothek_Name{Index, 1}];
            
            Part_3 = [Filename{FileName_Index,:}];
            
            Gesamtbefehl{Index} = append(Part_1, ' ', Part_2, ' ', Part_3);
        end
        Gesamtbefehl = Gesamtbefehl';
    end
    
    Gesamtbefehl_Alles = [Gesamtbefehl_Alles;Gesamtbefehl];
    
end

fid = fopen('Terminal_Commands.txt', 'w');
fprintf(fid, '%s\r\n\r\n', Gesamtbefehl_Alles{:});

%% Run the commands in the terminal
for i = 1:size(Gesamtbefehl_Alles, 1)
    system(Gesamtbefehl_Alles{i})
end

