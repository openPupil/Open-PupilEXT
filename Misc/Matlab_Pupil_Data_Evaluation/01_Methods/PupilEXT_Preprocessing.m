classdef PupilEXT_Preprocessing
    
    properties
    end
    
    methods
        
        function self = PupilEXT_Preprocessing()
            datetime.setDefaultFormats('default','dd.MM.yyyy HH:mm:ss.SSS');
        end
        
        %{
        Input: Raw pupil data recorded by PupilEXT
        Function:
        The timestamp will be transformed correctly (set offline flag)
        Unreliable data will be filtered
        %}
        function [clean_Table] = clean_Table(self, Pupil_Data, offline)
            % Set the standard time format
            datetime.setDefaultFormats('default','dd.MM.yyyy HH:mm:ss.SSS');
            
            if offline == 'true'
                % Filename: Remove the dot from the column and convert to number
                Pupil_Data = removevars(Pupil_Data, 'timestamp_ms');
                Splitted = split(Pupil_Data.filename(1:end), '.');
                Splitted = str2num(cell2mat(Splitted(:,1)));
                Pupil_Data.timestamp_ms_ = Splitted;
                Pupil_Data = removevars(Pupil_Data, 'filename');
                
                % Convert to the correct time format
                Pupil_Data.Datetime = datetime(Pupil_Data.timestamp_ms_/1000,...
                    'convertfrom','posixtime', 'TimeZone', 'Europe/Berlin');
                Pupil_Data = removevars(Pupil_Data, 'timestamp_ms_');
                
                % Remove unwanted coloumns
                Pupil_Data = removevars(Pupil_Data, {
                    'diameterMain_px', 'diameterSec_px',...
                    'undistortedDiameterMain_px', 'undistortedDiameterSec_px',...
                    'axisRatioMain', 'axisRatioSec',...
                    'centerMain_x', 'centerMain_y', 'centerSec_x', 'centerSec_y',...
                    'angleMain_deg', 'angleSec_deg',...
                    'circumferenceMain_px', 'circumferenceSec_px',...
                    'confidenceMain', 'confidenceSec'});
                
                % Added Axis ratio coloumn
                AxisRatio_Main = Pupil_Data.widthMain_px./Pupil_Data.heightMain_px;
                AxisRatio_Sec = Pupil_Data.widthSec_px./Pupil_Data.heightSec_px;
                
                Pupil_Data = removevars(Pupil_Data, {'widthMain_px', 'heightMain_px',...
                    'widthSec_px', 'heightSec_px'});
                
                Pupil_Data.AxisRatio_Main = AxisRatio_Main;
                Pupil_Data.AxisRatio_Sec = AxisRatio_Sec;
                
                % Sorted coloumns:  Datetime, AxisRatio_Main, AxisRatio_Sec,
                %   outline_confidence, outline_confidenceSec,physicaldiameter_mm_
                Pupil_Data = movevars(Pupil_Data,'outlineConfidenceMain','After','AxisRatio_Sec');
                Pupil_Data = movevars(Pupil_Data,'outlineConfidenceSec','After','outlineConfidenceMain');
                Pupil_Data = movevars(Pupil_Data,'physicalDiameter_mm','After','outlineConfidenceSec');
                Pupil_Data = movevars(Pupil_Data,'algorithm','After','physicalDiameter_mm');
                
                % Rename der coloumns
                Pupil_Data.Properties.VariableNames = {'Datetime', 'AxisRatio_Main', 'AxisRatio_Sec',...
                    'OutlineConfidence_Main', 'OutlineConfidence_Sec', 'PupilDiameterLeft', 'Algorithm'};
                
                % Cast to timetable format
                clean_Table = table2timetable(Pupil_Data);
            end
        end
        
        
        function [Pupil_Data_Clean, MarkerTable_New] = assign_Marker(self, Pupil_Data, Marker_Table)
            
            % Die zwei Zeitspalten als eine Spalte zusammenfügen
            Marker_Table.DateTime = datetime(Marker_Table.Datum,'InputFormat','MM/dd/yyyy',...
                'Format','dd.MM.yy HH:mm:ss.SSS', 'TimeZone', 'Europe/Zurich') + ...
                duration(Marker_Table.Uhrzeit,'InputFormat','hh:mm:ss.SSS','Format','hh:mm:ss.SSS');
            
            % Unnötige Spalten Datum und Uhrzeit entfernen
            Marker_Table = removevars(Marker_Table,{'Datum','Uhrzeit'});
            
            % Erste Zeile Initial rausnehmen
            Marker_Table(1,:) = [];
            
            % Zuweisen der SoectrumCode Marker zu den Pupillendaten
            % -1 da der Letzte SpectrumCode das Ende zuweisen soll und kein Code benötigt wird
            Pupil_Data.SpektrumCode = discretize(Pupil_Data.Datetime, Marker_Table.DateTime,...
                'categorical', Marker_Table.Spektrumcode(1:end-1));
            
            Pupil_Data.Spektrumbezeichnung = discretize(Pupil_Data.Datetime, Marker_Table.DateTime,...
                'categorical', Marker_Table.Spektrumbezeichnung(1:end-1));
            
            Pupil_Data.Versuchsbezeichnung = discretize(Pupil_Data.Datetime, Marker_Table.DateTime,...
                'categorical', Marker_Table.Versuchsbezeichnung_1(1:end-1));
            
            % Daten entfernen, welche keiner Kategorie zugehören
            Logik_Array = not(isundefined(Pupil_Data.Spektrumbezeichnung));
            Pupil_Data_Clean = Pupil_Data(Logik_Array,:);
            
            % Probandencode, Alter, Vorname, Alter in die Daten Pupillendaten hinzufügen
            Pupil_Data_Clean.Probandencode(:) = Marker_Table.Probandencode(1);
            %Pupil_Data_Clean.Vorname(:) = Marker_Table.Vorname(1);
            %Pupil_Data_Clean.Nachname(:) = Marker_Table.Nachname(1);
            Pupil_Data_Clean.Alter(:) = Marker_Table.Alter(1);
            
            % Verändern des Datentypes: Alter(numerical), Probandencode (categorical)
            Pupil_Data_Clean.Alter = str2num(cell2mat(Pupil_Data_Clean.Alter(:)));
            Pupil_Data_Clean.Probandencode = categorical(Pupil_Data_Clean.Probandencode(:));
            
            MarkerTable_New = Marker_Table;
        end
        
        function [Pupil_Data_RAW, Pupil_Data_Processed, Invalid_Data_Percent, BlinkCount, Struct_Plotting] = removeBlinks(self, Pupil_Data, Marker, FPS, Retime_HZ)
            
            % Neue Werte setzen für die weiterverarbeitung
            Pupil_Data_vorher = Pupil_Data;
            Pupil_Data_without_Blink = Pupil_Data;
            
            % Alle Daten mit einer Confidenz < 1 bei main und sec auf NaN setzen
            Pupil_Data_without_Blink.PupilDiameterLeft(Pupil_Data_without_Blink.OutlineConfidence_Main < 1 |...
                Pupil_Data_without_Blink.OutlineConfidence_Sec < 1) = NaN;
              
            % Tabelle erstellen mit Zeitpunkten an denen InValide Datenpunkte vorkommen
            % zum Markieren der invaliden Daten
            NaN_Table = Pupil_Data_without_Blink(isnan(Pupil_Data_without_Blink.PupilDiameterLeft), :);
            Pupil_Data_without_Blink.FilteredDiameter = isnan(Pupil_Data_without_Blink.PupilDiameterLeft);
            
            % Auch die Werte auf NaN setzen die +1 nach NaN kommen
            % Dazu wird die Tabelle genommen mit den NaN Werten und jeder Wert aus
            % der Pupil_Data_without_Blink Tabelle herausgezogen, um den Index zu bestimmen
            % Aus diesem Index wird + 1 gemacht, um den nächsten Wert in der Tabelle auf NaN zu setzen
            Threshold_NextFrame_ms = (1/30) * 1000;
            
            counter_index = 0;
            for NaN_Frame = 1:size(NaN_Table, 1)-1
                
                %disp(Pupil_Data_without_Blink(NaN_Table.Datetime(NaN_Frame), :))
                FrameTime_Diff = (posixtime(NaN_Table.Datetime(NaN_Frame+1)) - posixtime(NaN_Table.Datetime(NaN_Frame)))*1000;
                %disp(FrameTime_Diff)
                % Wenn das nächste Frame auch ein NaN hat, dann muss der nächste (3. Frame) auch ein NaN bekommen
                if FrameTime_Diff <= Threshold_NextFrame_ms + 1
                    % Überprüfen, ob es nicht schon beim letzten Frame ist, da es sonst kein drittes gibt
                    if NaN_Frame < size(NaN_Table, 1)-3
                        
                        % Jetzt den Zeitstempel vom dritten Frame nehmen und den Pupillendurchmesser auf NaN setzen
                        % Zusätzlich auch die Filer-Spalte setzen
                        %disp(Pupil_Data_without_Blink(NaN_Table.Datetime(NaN_Frame), :))
                        TimeOfIntest = Pupil_Data_without_Blink(NaN_Table.Datetime(NaN_Frame+1), :).Datetime;
                        Index = find(Pupil_Data_without_Blink.Datetime == TimeOfIntest);
                        
                        % Drei nach dem Wert auf NaN setzen
                        Pupil_Data_without_Blink(Index + 1, :).PupilDiameterLeft = NaN;
                        Pupil_Data_without_Blink(Index + 2, :).PupilDiameterLeft = NaN;
                        Pupil_Data_without_Blink(Index + 3, :).PupilDiameterLeft = NaN;
                        
                        if Index > 4
                            Pupil_Data_without_Blink(Index - 1, :).PupilDiameterLeft = NaN;
                            Pupil_Data_without_Blink(Index - 2, :).PupilDiameterLeft = NaN;
                            Pupil_Data_without_Blink(Index - 3, :).PupilDiameterLeft = NaN;
                        end
                    end
                end
                %counter_index = counter_index + 1;
                %fprintf('%d von %d \n', counter_index, size(NaN_Table, 1)-1)
            end
            
            % Wenn in einem Fenster von vier Werten alle ConfidenzValues true sind, dann liegt ein Blinzler vor
            Index_Array = zeros(size(Pupil_Data_without_Blink, 1), 1, 1);
            Window_Size = 4;
            Counter = 0;
            for Blink_Index = 2:size(Pupil_Data_without_Blink, 1)-3
                
                Vorheriges_Fenster = Pupil_Data_without_Blink(Blink_Index-1:Blink_Index+Window_Size-2, :);
                Aktuelles_Fenster = Pupil_Data_without_Blink(Blink_Index:Blink_Index+Window_Size-1, :);
                
                if sum(Aktuelles_Fenster.FilteredDiameter(:) == true) == 4
                    
                    if sum(Vorheriges_Fenster.FilteredDiameter(:) == true) == 4
                        Index_Array(Blink_Index:Blink_Index+Window_Size-1) = Counter;
                    else
                        Counter = Counter + 1;
                        Index_Array(Blink_Index:Blink_Index+Window_Size-1) = Counter;
                    end
                else
                    Index_Array(Blink_Index:Blink_Index+Window_Size-1) =...
                        (Index_Array(Blink_Index:Blink_Index+Window_Size-1) | zeros(4, 1, 1))*Counter;
                end
            end
            Pupil_Data_without_Blink.Blink_Count = Index_Array;
            
            % Zählen der NaN Werte in dem Datensatz in Prozent -> Das sollte für jede Kategorie gemacht werden ---            
            %Prozent_InValideDaten = size(Pupil_Data_without_Blink.PupilDiameterLeft(isnan(Pupil_Data_without_Blink.PupilDiameterLeft)), 1)*100/...
            %    size(Pupil_Data_without_Blink.PupilDiameterLeft, 1);
             Prozent_InValideDaten = table(splitapply(@(x) (size(x(isnan(x)), 1)*100)/size(x, 1), Pupil_Data_without_Blink.PupilDiameterLeft,...
                 findgroups(categorical(Pupil_Data_without_Blink.Spektrumbezeichnung))),...
                 splitapply(@(x) x(1), Pupil_Data_without_Blink.Spektrumbezeichnung,...
                 findgroups(categorical(Pupil_Data_without_Blink.Spektrumbezeichnung))),...
                 'VariableNames', {'Invalid_Data_Percent' 'Spektrumsbezeichnung'});
            % ----------------------------------------------------------------------------------------------------

            
            % Ersetzen der fehlenden Werte durch lineare Interpolation
            Pupil_Data_without_Blink.PupilDiameterLeft = fillmissing(Pupil_Data_without_Blink.PupilDiameterLeft,...
                'linear','SamplePoints', Pupil_Data_without_Blink.Datetime);
            
            % Filtern über das Achsenverhältnis
            % Zunächst muss die Differenz gebildet werden zwischen den Achsenverhältnissen
            Pupil_Data_without_Blink_and_Outlier = Pupil_Data_without_Blink;
            Axis_Differenz = abs(Pupil_Data_without_Blink_and_Outlier.AxisRatio_Main - Pupil_Data_without_Blink_and_Outlier.AxisRatio_Sec);
            Pupil_Data_without_Blink_and_Outlier.Axis_Differenz = Axis_Differenz;
            Pupil_Data_without_Blink_and_Outlier = Pupil_Data_without_Blink_and_Outlier(~isoutlier(Pupil_Data_without_Blink_and_Outlier.Axis_Differenz,'percentiles', [5 95]), :);
            
            
            % Onset und Offset vom Liedschlag erhalten -----------------------------
            Daten_Blinks = Pupil_Data_without_Blink;
            Daten_Blinks.FilteredDiameter = categorical(uint8(Daten_Blinks.Blink_Count));
            Onset_Offset_Blink = splitapply(@(x) [x(1) x(end)], Daten_Blinks.Datetime, findgroups(Daten_Blinks.Blink_Count));
            Pupil_onset_Table = Pupil_Data_vorher(Onset_Offset_Blink(:,1),:);
            Pupil_offset_Table = Pupil_Data_vorher(Onset_Offset_Blink(:,2),:);
            % ----------------------------------------------------------------------
            
            
            % Plotten der Pupilldaten Vorher / Nachher -----------------------------------
            figure;
            set(gcf, 'Position', [0.1, 11, 7, 5]);
            tiledlayout(2, 1, 'TileSpacing', 'compact', 'Padding','compact');
            nexttile;
            plot(Pupil_Data_vorher.Datetime, Pupil_Data_vorher.PupilDiameterLeft);
            xlim([min(Pupil_Data_vorher.Datetime)...
                max(Pupil_Data_vorher.Datetime)]);
            xlabel('Time')
            ylabel('Pupil diameter in mm')
            box off; grid off;
            gcaObj = gca;
            gcaObj.LineWidth=1;
            set(gcaObj,'TickDir','out');
            gcaObj.XAxis.TickLength = [0.0150 0.0250];
            gcaObj.YAxis.TickLength = [0.0150 0.0250];
            set(gcaObj, 'YLim', [1 7], 'YTick', [1:2:7], 'YMinorTick','off');
            %title('Link VORHER mit Blinks')
            
            % Grenzen markieren für die verschiedenen Spekktren im Plot
            hold on;
            T = Marker;
            
            for i = 1:2:(size(T,1)-1)
                fill( [ T.DateTime(i,1)  T.DateTime(i,1) T.DateTime(i+1,1) T.DateTime(i+1,1) ], [1 7 7 1], 'y', 'FaceAlpha', 0.05,'EdgeColor','none');
                fill( [ T.DateTime(i+1,1)  T.DateTime(i+1,1) T.DateTime(i+2,1) T.DateTime(i+2,1) ], [1 7 7 1], 'b', 'FaceAlpha', 0.05,'EdgeColor','none');
            end
            scatter(Pupil_onset_Table.Datetime, Pupil_onset_Table.PupilDiameterLeft, 10,...
                'MarkerEdgeColor',[0.4660 0.6740 0.1880], 'MarkerFaceColor',[0.4660 0.6740 0.1880], 'LineWidth', 0.1);
            scatter(Pupil_offset_Table.Datetime, Pupil_offset_Table.PupilDiameterLeft, 10,...
                'MarkerEdgeColor',[0.6350 0.0780 0.1840], 'MarkerFaceColor',[0.6350 0.0780 0.1840], 'LineWidth', 0.1);
            hold off;
            
            nexttile;
            plot(Pupil_Data_without_Blink_and_Outlier.Datetime, Pupil_Data_without_Blink_and_Outlier.PupilDiameterLeft);
            xlim([min(Pupil_Data_without_Blink_and_Outlier.Datetime) max(Pupil_Data_without_Blink_and_Outlier.Datetime)]);
            xlabel('Time')
            ylabel('Pupil diameter in mm')
            box off; grid off;
            gcaObj = gca;
            gcaObj.LineWidth=1;
            set(gcaObj,'TickDir','out');
            gcaObj.XAxis.TickLength = [0.0150 0.0250];
            gcaObj.YAxis.TickLength = [0.0150 0.0250];
            set(gcaObj, 'YLim', [1 7], 'YTick', [1:2:7], 'YMinorTick','off');
             
            % Grenzen markieren für die verschiedenen Spekktren im Plot
            hold on;
            T = Marker;
            for i = 1:2:(size(T,1)-1)
                fill( [ T.DateTime(i,1)  T.DateTime(i,1) T.DateTime(i+1,1) T.DateTime(i+1,1) ], [1 7 7 1], 'y', 'FaceAlpha', 0.05,'EdgeColor','none');
                fill( [ T.DateTime(i+1,1)  T.DateTime(i+1,1) T.DateTime(i+2,1) T.DateTime(i+2,1) ], [1 7 7 1], 'b', 'FaceAlpha', 0.05,'EdgeColor','none');
            end
            hold off;
            
            % Entfernen der Spalten, die nicht mehr benötigt werden
            % Zusatz muss erst mal nichts entfernt werden, sieht gut aus
            
            % Tabelle über den report zurück geben, Anzahl der Invaliden Daten bzw. Validen Daten
            Anzahl_Blink = max(unique(Pupil_Data_without_Blink_and_Outlier.Blink_Count(Pupil_Data_without_Blink_and_Outlier.Blink_Count > 0) -...
                min(Pupil_Data_without_Blink_and_Outlier.Blink_Count(Pupil_Data_without_Blink_and_Outlier.Blink_Count > 0))) + 1);
            
            %disp(table(Anzahl_Blink, Prozent_InValideDaten, 'VariableNames', {'Blink Anzahl', 'Invalide Daten [%]'}))
            
            % Retime to 20 Hz for each Categorie - Übergabe
            % Schon hier muss genullt werden, da es sonst Probleme beim Umschalten gibt und die Werte nicht gerade sind
            
            % Zurücksetzen der Datetime auf null
            Pupil_Data_without_Blink_and_Outlier.Datetime = datetime(posixtime(Pupil_Data_without_Blink_and_Outlier.Datetime) -  posixtime(Pupil_Data_without_Blink_and_Outlier.Datetime(1)),...
                'convertfrom', 'posixtime', 'Format', 'dd.MM.yyyy HH:mm:ss.SSS', 'TimeZone', 'Europe/Zurich');
            
            % Messen wielange eine Kategorie gedauert hat und Runden
            Round_times = cell2mat(splitapply(@(x) {round(seconds(x(end)-x(1)))},...
                Pupil_Data_without_Blink_and_Outlier.Datetime, findgroups(Pupil_Data_without_Blink_and_Outlier.SpektrumCode)));
            
            % Startzeitpunkt aus dem Datensatz ziehen
            Startzeitpunkt = Pupil_Data_without_Blink_and_Outlier.Datetime(1);
            
            % Erhalten der Kategorien
            Kategorien = cell2mat(categories(Pupil_Data_without_Blink_and_Outlier.SpektrumCode));
            
            for i = 1:size(Kategorien, 1)
                
                Filter_Data = Pupil_Data_without_Blink_and_Outlier(Pupil_Data_without_Blink_and_Outlier.SpektrumCode == Kategorien(i),:);
                
                Endzeitpunkt = Startzeitpunkt + seconds(Round_times(i));
                
                % Achtung nicht mehr 100 Hz, es ist eine andere Zeit
                New_Time_100Hz = (Startzeitpunkt:milliseconds((1/Retime_HZ)*1000):Endzeitpunkt)';
                
                %                 datetime(posixtime(New_Time_100Hz) -  posixtime(New_Time_100Hz(1)),...
                %                     'convertfrom', 'posixtime', 'Format', 'dd.MM.yyyy HH:mm:ss.SSS', 'TimeZone', 'Europe/Zurich')
                
                Filter_Data_retimed = retime(Filter_Data, New_Time_100Hz, 'nearest');
                
                Startzeitpunkt = Filter_Data_retimed.Datetime(end);
                
                if i == 1
                    PufferData = Filter_Data_retimed;
                else
                    PufferData = [PufferData; Filter_Data_retimed];
                end
                
                %                 datetime(posixtime(PufferData.Datetime) -  posixtime(PufferData.Datetime(1)),...
                %                     'convertfrom', 'posixtime', 'Format', 'dd.MM.yyyy HH:mm:ss.SSS', 'TimeZone', 'Europe/Zurich')
            end
            Pupil_Data_without_Blink_and_Outlier = PufferData;
            
            % Checken was passiert
            Function = @(x) {x(1)};
            Alt = splitapply(Function, Pupil_Data_vorher.Datetime, findgroups(Pupil_Data_vorher.SpektrumCode));
            Neu = splitapply(Function, Pupil_Data_without_Blink_and_Outlier.Datetime, findgroups(Pupil_Data_without_Blink_and_Outlier.SpektrumCode));
            %disp(table(Alt, Neu));
            
            Pupil_Data_without_Blink_and_Outlier = removevars(Pupil_Data_without_Blink_and_Outlier,...
                {'AxisRatio_Main', 'AxisRatio_Sec', 'OutlineConfidence_Main',...
                'OutlineConfidence_Sec', 'FilteredDiameter', 'Blink_Count', 'Axis_Differenz'});
           
            % ----------------------------
            Struct_Plotting.Pupil_Data_vorher = Pupil_Data_vorher;
            Struct_Plotting.T = T;
            Struct_Plotting.Pupil_onset_Table = Pupil_onset_Table;
            Struct_Plotting.Pupil_offset_Table = Pupil_offset_Table;
            Struct_Plotting.Pupil_Data_without_Blink_and_Outlier = Pupil_Data_without_Blink_and_Outlier;      
            % ----------------------------
         
            Pupil_Data_RAW = Pupil_Data_vorher;
            Pupil_Data_Processed = Pupil_Data_without_Blink_and_Outlier;
            Invalid_Data_Percent = Prozent_InValideDaten;
            BlinkCount = Anzahl_Blink;  
        end
        
    end
end