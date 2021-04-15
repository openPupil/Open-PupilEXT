%% Information
% Author: Babak Zandi
% Technical University of Darmstadt
% Laboratory of Lighting Technology
%
% This code is a supplementary material from the work
% PupilEXT: flexible open-source platform for high resolution
% pupillometry in vision reseach

%% Plot adjustments
set(groot, 'DefaultLineLineWidth', 2);
set(groot, 'DefaultAxesLineWidth', 1);
set(groot, 'DefaultAxesFontName', 'Charter');
set(groot, 'DefaultAxesFontSize', 7);
set(groot, 'DefaultAxesFontWeight', 'normal');
set(groot, 'DefaultAxesXMinorTick', 'on');
set(groot, 'DefaultAxesXGrid', 'on');
set(groot, 'DefaultAxesYGrid', 'on');
set(groot, 'DefaultAxesGridLineStyle', ':');
set(groot, 'DefaultAxesUnits', 'normalized');
set(groot, 'DefaultAxesOuterPosition',[0, 0, 1, 1]);
set(groot, 'DefaultFigureUnits', 'inches');
set(groot, 'DefaultFigurePaperPositionMode', 'manual');
set(groot, 'DefaultFigurePosition', [0.1, 11, 8.5, 4.5]);
set(groot, 'DefaultFigurePaperUnits', 'inches');
set(groot, 'DefaultFigurePaperPosition', [0.1, 11, 8.5, 4.5]);

%% Add folder to path
addpath('01_Methods')
addpath('00_Data/')
addpath('00_Data/Marker_Tables/')
addpath('00_Data/Processed_Data/')
addpath('00_Data/Pupil_Log_Data/')
addpath('00_Data/Pupil_Log_Data/00_Else/')
addpath('00_Data/Pupil_Log_Data/01_ExCuSe/')
addpath('00_Data/Pupil_Log_Data/02_PuRe/')
addpath('00_Data/Pupil_Log_Data/03_PuReST/')
addpath('00_Data/Pupil_Log_Data/04_Starbust/')
addpath('00_Data/Pupil_Log_Data/05_Swirski2D/')
addpath('00_Data/Pupil_Log_Data/00_Else/00_ROI_0_3')
addpath('00_Data/Pupil_Log_Data/01_ExCuSe/00_ROI_0_3')
addpath('00_Data/Pupil_Log_Data/02_PuRe/00_ROI_0_3')
addpath('00_Data/Pupil_Log_Data/03_PuReST/00_ROI_0_3')
addpath('00_Data/Pupil_Log_Data/04_Starbust/00_ROI_0_3')
addpath('00_Data/Pupil_Log_Data/05_Swirski2D/00_ROI_0_3')

%% Load pupil data and marker tables into matlab

clc; clear; clear; close all;
Dateinamen(1).Daten_Namen = dir('00_Data/Marker_Tables/*.mat');
Experiment_Name = 'Const_L_Quasi';

% Load marker tables into matlab
MarkerTable_Data = struct();
for Index = 1: length(Dateinamen(1).Daten_Namen)
    load(Dateinamen(1).Daten_Namen(Index).name);
    
    Dateiname = [T.Probandencode{1} '_Marker' '_' Experiment_Name];
    
    assignin('base', (Dateiname), T);
    MarkerTable_Data.(Dateiname) = evalin('base', Dateiname);
    clear(Dateiname)
end

clearvars Dateiname Dateinamen Index T ans

% Load pupil raw data into matlab
Dateinamen(1).Daten_Namen = dir('00_Data/Pupil_Log_Data/00_Else/00_ROI_0_3/*.csv');     % Pupil data  "00_Else"
Dateinamen(2).Daten_Namen = dir('00_Data/Pupil_Log_Data/01_ExCuSe/00_ROI_0_3/*.csv');   % Pupil data  "01_ExCuSe"
Dateinamen(3).Daten_Namen = dir('00_Data/Pupil_Log_Data/02_PuRe/00_ROI_0_3/*.csv');     % Pupil data  "02_PuRe"
Dateinamen(4).Daten_Namen = dir('00_Data/Pupil_Log_Data/03_PuReST/00_ROI_0_3/*.csv');   % Pupil data  "03_PuReST"
Dateinamen(5).Daten_Namen = dir('00_Data/Pupil_Log_Data/04_Starbust/00_ROI_0_3/*.csv'); % Pupil data  "04_Starbust"
Dateinamen(6).Daten_Namen = dir('00_Data/Pupil_Log_Data/05_Swirski2D/00_ROI_0_3/*.csv');% Pupil data  "05_Swirski2D"

TrialNames = {'A', 'B', 'C', 'D', 'E', 'F'};
Array_Algo_Names = {'Else', 'ExCuSe', 'PuRe', 'PuReST', 'Starbust', 'Swirski2D'};
PupilRaw_Data = struct();

for FolderIndex = 1:6
    for FileIndex = 1:length(Dateinamen(1).Daten_Namen)
        Filename = Dateinamen(FolderIndex).Daten_Namen(FileIndex).name;
        Splitted_Filename = split(Filename,'_');
        
        AlgorithmName = Splitted_Filename{3};
        TrialName = strsplit(Splitted_Filename{6}, '.');
        
        New_Filename = [TrialName{1} '_PupilRaw_' AlgorithmName];
        
        T = readtable(Filename, 'FileType','text');
        
        assignin('base', (New_Filename), T)
        
        if FolderIndex == 1
            PupilRaw_Data.Else.(New_Filename) = evalin('base', New_Filename);
            clear(New_Filename)
        elseif FolderIndex == 2
            PupilRaw_Data.ExCuSe.(New_Filename) = evalin('base', New_Filename);
            clear(New_Filename)
        elseif FolderIndex == 3
            PupilRaw_Data.PuRe.(New_Filename) = evalin('base', New_Filename);
            clear(New_Filename)
        elseif FolderIndex == 4
            PupilRaw_Data.PuReST.(New_Filename) = evalin('base', New_Filename);
            clear(New_Filename)
        elseif FolderIndex == 5
            PupilRaw_Data.Starbust.(New_Filename) = evalin('base', New_Filename);
            clear(New_Filename)
        elseif FolderIndex == 6
            PupilRaw_Data.Swirski2D.(New_Filename) = evalin('base', New_Filename);
            clear(New_Filename)
        end
    end
end

clearvars TrialNames TrialName T Splitted_Filename New_Filename FileIndex...
    FolderIndex Filename Experiment_Name Dateinamen Array_Algo_Names AlgorithmName

%% Preprocess and synch pupil data
AlgoNames = fieldnames(PupilRaw_Data);
MarkerTableNames = fieldnames(MarkerTable_Data);
TrialNames = {'A', 'B', 'C', 'D', 'E', 'F'};

PupilEXT_Object = PupilEXT_Preprocessing();
PupilSmooth_Data = struct();
Plotting_Struct = struct();

Counter = 0;

Blink_Count_T = [];
Invalid_Data_Percent_T = [];
Trial_T = [];
Algorithm_T = [];
NonVolatile_Table = table();


for AlgoIndex = 1:length(AlgoNames)
    
    PupilRawFieldNames = fieldnames(PupilRaw_Data.(AlgoNames{AlgoIndex}));
    MarkerTableFieldNames = fieldnames(MarkerTable_Data);
    
    for TrialIndex = 1:length(PupilRawFieldNames)
        
        Current_PupilRaw_Table = PupilRaw_Data.(AlgoNames{AlgoIndex}).(PupilRawFieldNames{TrialIndex});
        Current_Marker_Table = MarkerTable_Data.(MarkerTableFieldNames{TrialIndex});
        
        Pupil_Data = PupilEXT_Object.clean_Table(Current_PupilRaw_Table, 'true');
        [Pupil_Data, Marker_Table] = PupilEXT_Object.assign_Marker(Pupil_Data, Current_Marker_Table);
        [Pupil_Data, Pupil_Data_Processed, Invalid_Data_Percent, BlinkCount, Struct_Plotting] =...
            PupilEXT_Object.removeBlinks(Pupil_Data, Marker_Table, 30, 20);
        
        Splitted_Val_Name = split(PupilRawFieldNames{TrialIndex},'_');
        Splitted_Val_Name{2} = 'PupilSmooth';
        New_Name = strjoin(Splitted_Val_Name,'_');
        assignin('base', (New_Name), Pupil_Data_Processed)
        
        PupilSmooth_Data.(AlgoNames{AlgoIndex}).(New_Name) = evalin('base', New_Name);
        
        Plotting_Struct.(AlgoNames{AlgoIndex}).(New_Name).Struct_Plotting = Struct_Plotting;
        
        clear(New_Name)
        
        Counter = Counter + 1;
        
        Blink_Count_T(Counter) = BlinkCount;
        Trial_T{Counter} = MarkerTableFieldNames{TrialIndex};
        Algorithm_T{Counter} = AlgoNames{AlgoIndex};
        
        % Table for the invalid data in percent
        Volatile_Table = Invalid_Data_Percent;
        Trial_Letter = split(PupilRawFieldNames{TrialIndex}, '_');
        Volatile_Table.Trial(:) = Trial_Letter(1);
        Volatile_Table.Spektrumsbezeichnung = cellstr(Volatile_Table.Spektrumsbezeichnung);
        Volatile_Table.Algorithm(:) =  AlgoNames(AlgoIndex);
        
        NonVolatile_Table = [NonVolatile_Table; Volatile_Table];
        
        fprintf('Round [%d / %d] ', Counter, length(PupilRawFieldNames)*length(AlgoNames))
        disp(['Blink Count: ' num2str(BlinkCount) ' -- Pupil Raw: ' PupilRawFieldNames{TrialIndex} ' <--> ' 'Marker: ' MarkerTableFieldNames{TrialIndex}])
    end
end

Summary_Performance = table(Blink_Count_T', Trial_T', Algorithm_T', 'VariableNames',...
    {'Blink_Count', 'Trial', 'Algorithm'});

PupilSmooth_Data.Summary_Performance = Summary_Performance;
PupilSmooth_Data.Invalid_Data_Performance = NonVolatile_Table;

clearvars AlgoIndex AlgoNames BlinkCount Counter Current_Marker_Table Current_PupilRaw_Table...
    Invalid_Data_Percent Marker_Table MarkerTableFieldNames MarkerTableNames New_Name Pupil_Data...
    Pupil_Data_New Pupil_Data_Processed PupilEXT_Object PupilRawFieldNames Splitted_Val_Name TrialIndex TrialNames...
    Blink_Count_T Invalid_Data_Percent_T Trial_T Algorithm_T Summary_Performance Pupil_Data_Processed Struct_Plotting...
    NonVolatile_Table Trial_Letter Volatile_Table

save('00_Data/Pupil_Data')

%%  Equalize pupil data order as there were recorded in random spectra order
% Export preferences:
clc; clear; close all;
load('00_Data/Pupil_Data')

datetime.setDefaultFormats('default','dd.MM.yyyy HH:mm:ss.SSS')
AlgoNames = fieldnames(rmfield(PupilSmooth_Data, {'Summary_Performance', 'Invalid_Data_Performance'}));

for AlgoIndex = 1:length(AlgoNames)
    
    PupilFieldNames = fieldnames(PupilSmooth_Data.(AlgoNames{AlgoIndex}));
    
    for TrialIndex = 1:length(PupilFieldNames)
        
        Current_Pupil_Table = PupilSmooth_Data.(AlgoNames{AlgoIndex}).(PupilFieldNames{TrialIndex});
        
        Zeitachse = datetime(posixtime(Current_Pupil_Table.Datetime) -  posixtime(Current_Pupil_Table.Datetime(1)),...
            'convertfrom', 'posixtime', 'Format', 'dd.MM.yyyy HH:mm:ss.SSS', 'TimeZone', 'Europe/Zurich');
        
        Grenzen = splitapply(@(x) {x(1)}, Current_Pupil_Table.Datetime, findgroups(Current_Pupil_Table.SpektrumCode));
        
        Current_Pupil_Table.SpektrumCode = cellstr(Current_Pupil_Table.SpektrumCode(:));
        Current_Pupil_Table = sortrows(Current_Pupil_Table,{'SpektrumCode'});
        
        Current_Pupil_Table.SpektrumCode = categorical(Current_Pupil_Table.SpektrumCode);
        
        Current_Pupil_Table.Datetime = Zeitachse;
        
        PupilSmooth_Data.(AlgoNames{AlgoIndex}).(PupilFieldNames{TrialIndex}) = Current_Pupil_Table;
        
    end
    
end

clearvars AlgoIndex AlgoNames Current_Pupil_Table Grenzen PupilFieldNames TrialIndex Zeitachse

% Cast all data to long format
AlgoNames = fieldnames(rmfield(PupilSmooth_Data, {'Summary_Performance', 'Invalid_Data_Performance'}));

for AlgoIndex = 1:length(AlgoNames)
    
    PupilFieldNames = fieldnames(PupilSmooth_Data.(AlgoNames{AlgoIndex}));
    PupilSmooth_Placeholder = struct();
    
    for TrialIndex = 1:length(PupilFieldNames)
        
        Current_Pupil_Table = PupilSmooth_Data.(AlgoNames{AlgoIndex}).(PupilFieldNames{TrialIndex});
        
        Current_Pupil_Table.DateTime_Reset_Zero = cell2mat(splitapply(@(x) {posixtime(x) - posixtime(x(1))},...
            Current_Pupil_Table.Datetime, findgroups(Current_Pupil_Table.SpektrumCode)));
        
        Current_Pupil_Table.DateTime_Reset_Zero = datetime(Current_Pupil_Table.DateTime_Reset_Zero,...
            'convertfrom', 'posixtime', 'Format', 'dd.MM.yyyy HH:mm:ss.SSS', 'TimeZone', 'Europe/Zurich');
        
        % ----------------------------------
        Current_Pupil_Table.SpektrumCode = cellstr(Current_Pupil_Table.SpektrumCode(:));
        Current_Pupil_Table.Spektrumbezeichnung = cellstr(Current_Pupil_Table.Spektrumbezeichnung(:));
        Current_Pupil_Table.Versuchsbezeichnung = cellstr(Current_Pupil_Table.Versuchsbezeichnung(:));
        Current_Pupil_Table.Probandencode = cellstr(Current_Pupil_Table.Probandencode(:));
        % ----------------------------------
        
        % Wieder zurückwandeln
        Current_Pupil_Table.SpektrumCode =  categorical(Current_Pupil_Table.SpektrumCode(:));
        Current_Pupil_Table.Spektrumbezeichnung = categorical(Current_Pupil_Table.Spektrumbezeichnung(:));
        Current_Pupil_Table.Versuchsbezeichnung = categorical(Current_Pupil_Table.Versuchsbezeichnung(:));
        Current_Pupil_Table.Probandencode = categorical(Current_Pupil_Table.Probandencode(:));
        % ----------------------------------
        
        PupilSmooth_Placeholder.(PupilFieldNames{TrialIndex}) = Current_Pupil_Table;
        
    end
    
    ZwischenVariable = struct2cell(PupilSmooth_Placeholder);
    Long_PupilSmooth = vertcat(ZwischenVariable{:});
    
    Splitted_Array = split((PupilFieldNames{TrialIndex}), '_');
    
    New_Name = ['Long_PupilSmooth_' Splitted_Array{3}];
    assignin('base', (New_Name), Long_PupilSmooth)
    
end

clearvars AlgoIndex AlgoNames Current_Pupil_Table Long_PupilSmooth New_Name...
    PupilFieldNames PupilSmooth_Placeholder Splitted_Array TrialIndex ZwischenVariable

save('00_Data/Pupil_Data')

%% Plotting A): Raw data from one trial for each algorithm, compared to its processed data
% Export settings - FontSize: 8, LinwWidth: 1,  Width: 16 cm, Height: 9
clc; clear; close all;
load('00_Data/Pupil_Data')

% Presets for plotting
fig = figure;
set(gcf, 'Position', [0.1, 11, 12, 7]);
t = tiledlayout(4, 3, 'TileSpacing', 'compact', 'Padding','tight');

% ----------------------------------------------------------------------------------------
% ELSE -----------------------------------------------------------------------------------
Current_Data = Plotting_Struct.Else.A_PupilSmooth_ElSe.Struct_Plotting;
Algorithm_Name = 'ElSe';

% Plot 1
nexttile(1);
plot(Current_Data.Pupil_Data_vorher.Datetime, Current_Data.Pupil_Data_vorher.PupilDiameterLeft);
xlim([min(Current_Data.Pupil_Data_vorher.Datetime)...
    max(Current_Data.Pupil_Data_vorher.Datetime)]);
box off; grid off;
gcaObj = gca;
gcaObj.LineWidth=1;
set(gcaObj,'TickDir','out');
gcaObj.XAxis.TickLength = [0.0150 0.0250];
gcaObj.YAxis.TickLength = [0.0150 0.0250];
set(gcaObj, 'YLim', [1 7], 'YTick', [1:2:7], 'YMinorTick','off');
set(gcaObj, 'XTick', [Current_Data.Pupil_Data_vorher.Datetime(1) Current_Data.Pupil_Data_vorher.Datetime(1)+seconds(90)...
    Current_Data.Pupil_Data_vorher.Datetime(1)+seconds(120) Current_Data.Pupil_Data_vorher.Datetime(1)+seconds(210)...
    Current_Data.Pupil_Data_vorher.Datetime(1)+seconds(240)]);
xtickformat('s')
xticklabels({'0', '90', '120', '210', '240'})
hold on;

for i = 1:2:(size(Current_Data.T,1)-1)
    fill( [ Current_Data.T.DateTime(i,1)  Current_Data.T.DateTime(i,1) Current_Data.T.DateTime(i+1,1) Current_Data.T.DateTime(i+1,1) ], [1 7 7 1], 'y', 'FaceAlpha', 0.05,'EdgeColor','none');
    fill( [ Current_Data.T.DateTime(i+1,1)  Current_Data.T.DateTime(i+1,1) Current_Data.T.DateTime(i+2,1) Current_Data.T.DateTime(i+2,1) ], [1 7 7 1], 'b', 'FaceAlpha', 0.05,'EdgeColor','none');
end
scatter(Current_Data.Pupil_onset_Table.Datetime, Current_Data.Pupil_onset_Table.PupilDiameterLeft, 10,...
    'MarkerEdgeColor',[0.4660 0.6740 0.1880], 'MarkerFaceColor',[0.4660 0.6740 0.1880], 'LineWidth', 0.1);
scatter(Current_Data.Pupil_offset_Table.Datetime, Current_Data.Pupil_offset_Table.PupilDiameterLeft, 10,...
    'MarkerEdgeColor',[0.6350 0.0780 0.1840], 'MarkerFaceColor',[0.6350 0.0780 0.1840], 'LineWidth', 0.1);
title([Algorithm_Name ': Raw Data']);
set(gca, 'XColor', [0 0 0])
set(gca, 'YColor', [0 0 0])
hold off;

% Plot 2
nexttile(4);
plot(Current_Data.Pupil_Data_without_Blink_and_Outlier.Datetime, Current_Data.Pupil_Data_without_Blink_and_Outlier.PupilDiameterLeft);
xlim([min(Current_Data.Pupil_Data_without_Blink_and_Outlier.Datetime) max(Current_Data.Pupil_Data_without_Blink_and_Outlier.Datetime)]);
box off; grid off;
gcaObj = gca;
gcaObj.LineWidth=1;
set(gcaObj,'TickDir','out');
gcaObj.XAxis.TickLength = [0.0150 0.0250];
gcaObj.YAxis.TickLength = [0.0150 0.0250];
set(gcaObj, 'YLim', [1 7], 'YTick', [1:2:7], 'YMinorTick','off');
set(gcaObj, 'XTick', [Current_Data.Pupil_Data_without_Blink_and_Outlier.Datetime(1) Current_Data.Pupil_Data_without_Blink_and_Outlier.Datetime(1)+seconds(90)...
    Current_Data.Pupil_Data_without_Blink_and_Outlier.Datetime(1)+seconds(120) Current_Data.Pupil_Data_without_Blink_and_Outlier.Datetime(1)+seconds(210)...
    Current_Data.Pupil_Data_without_Blink_and_Outlier.Datetime(1)+seconds(240)]);
xtickformat('s')
xticklabels({'0', '90', '120', '210', '240'})

hold on
Grenzwerte = splitapply(@(x) x(1), Current_Data.Pupil_Data_without_Blink_and_Outlier.Datetime,...
    findgroups(Current_Data.Pupil_Data_without_Blink_and_Outlier.SpektrumCode));

fill([Grenzwerte(1) Grenzwerte(1) Grenzwerte(2) Grenzwerte(2)],...
    [1 7 7 1], 'y', 'FaceAlpha', 0.05,'EdgeColor','none');
fill([Grenzwerte(2) Grenzwerte(2) Grenzwerte(3) Grenzwerte(3)],...
    [1 7 7 1], 'b', 'FaceAlpha', 0.05,'EdgeColor','none');
fill([Grenzwerte(3) Grenzwerte(3) Grenzwerte(4) Grenzwerte(4)],...
    [1 7 7 1], 'y', 'FaceAlpha', 0.05,'EdgeColor','none');
fill([Grenzwerte(4) Grenzwerte(4) Grenzwerte(4)+seconds(30) Grenzwerte(4)+seconds(30)],...
    [1 7 7 1], 'b', 'FaceAlpha', 0.05,'EdgeColor','none');
title([Algorithm_Name ': Processed Data']);
set(gca, 'XColor', [0 0 0])
set(gca, 'YColor', [0 0 0])
hold off;
% ELSE -----------------------------------------------------------------------------------
% ----------------------------------------------------------------------------------------


% ----------------------------------------------------------------------------------------
% ExCuSe -----------------------------------------------------------------------------------
Current_Data = Plotting_Struct.ExCuSe.A_PupilSmooth_ExCuSe.Struct_Plotting;
Algorithm_Name = 'ExCuSe';

% Plot 1
nexttile(2);
plot(Current_Data.Pupil_Data_vorher.Datetime, Current_Data.Pupil_Data_vorher.PupilDiameterLeft);
xlim([min(Current_Data.Pupil_Data_vorher.Datetime)...
    max(Current_Data.Pupil_Data_vorher.Datetime)]);
box off; grid off;
gcaObj = gca;
gcaObj.LineWidth=1;
set(gcaObj,'TickDir','out');
gcaObj.XAxis.TickLength = [0.0150 0.0250];
gcaObj.YAxis.TickLength = [0.0150 0.0250];
set(gcaObj, 'YLim', [1 7], 'YTick', [1:2:7], 'YMinorTick','off');
set(gcaObj, 'XTick', [Current_Data.Pupil_Data_vorher.Datetime(1) Current_Data.Pupil_Data_vorher.Datetime(1)+seconds(90)...
    Current_Data.Pupil_Data_vorher.Datetime(1)+seconds(120) Current_Data.Pupil_Data_vorher.Datetime(1)+seconds(210)...
    Current_Data.Pupil_Data_vorher.Datetime(1)+seconds(240)]);
xtickformat('s')
xticklabels({'0', '90', '120', '210', '240'})
hold on;

for i = 1:2:(size(Current_Data.T,1)-1)
    fill( [ Current_Data.T.DateTime(i,1)  Current_Data.T.DateTime(i,1) Current_Data.T.DateTime(i+1,1) Current_Data.T.DateTime(i+1,1) ], [1 7 7 1], 'y', 'FaceAlpha', 0.05,'EdgeColor','none');
    fill( [ Current_Data.T.DateTime(i+1,1)  Current_Data.T.DateTime(i+1,1) Current_Data.T.DateTime(i+2,1) Current_Data.T.DateTime(i+2,1) ], [1 7 7 1], 'b', 'FaceAlpha', 0.05,'EdgeColor','none');
end
scatter(Current_Data.Pupil_onset_Table.Datetime, Current_Data.Pupil_onset_Table.PupilDiameterLeft, 10,...
    'MarkerEdgeColor',[0.4660 0.6740 0.1880], 'MarkerFaceColor',[0.4660 0.6740 0.1880], 'LineWidth', 0.1);
scatter(Current_Data.Pupil_offset_Table.Datetime, Current_Data.Pupil_offset_Table.PupilDiameterLeft, 10,...
    'MarkerEdgeColor',[0.6350 0.0780 0.1840], 'MarkerFaceColor',[0.6350 0.0780 0.1840], 'LineWidth', 0.1);
title([Algorithm_Name ': Raw Data']);
set(gca, 'XColor', [0 0 0])
set(gca, 'YColor', [0 0 0])
hold off;

% Plot 2
nexttile(5);
plot(Current_Data.Pupil_Data_without_Blink_and_Outlier.Datetime, Current_Data.Pupil_Data_without_Blink_and_Outlier.PupilDiameterLeft);
xlim([min(Current_Data.Pupil_Data_without_Blink_and_Outlier.Datetime) max(Current_Data.Pupil_Data_without_Blink_and_Outlier.Datetime)]);
box off; grid off;
gcaObj = gca;
gcaObj.LineWidth=1;
set(gcaObj,'TickDir','out');
gcaObj.XAxis.TickLength = [0.0150 0.0250];
gcaObj.YAxis.TickLength = [0.0150 0.0250];
set(gcaObj, 'YLim', [1 7], 'YTick', [1:2:7], 'YMinorTick','off');
set(gcaObj, 'XTick', [Current_Data.Pupil_Data_without_Blink_and_Outlier.Datetime(1) Current_Data.Pupil_Data_without_Blink_and_Outlier.Datetime(1)+seconds(90)...
    Current_Data.Pupil_Data_without_Blink_and_Outlier.Datetime(1)+seconds(120) Current_Data.Pupil_Data_without_Blink_and_Outlier.Datetime(1)+seconds(210)...
    Current_Data.Pupil_Data_without_Blink_and_Outlier.Datetime(1)+seconds(240)]);
xtickformat('s')
xticklabels({'0', '90', '120', '210', '240'})

hold on
Grenzwerte = splitapply(@(x) x(1), Current_Data.Pupil_Data_without_Blink_and_Outlier.Datetime,...
    findgroups(Current_Data.Pupil_Data_without_Blink_and_Outlier.SpektrumCode));

fill([Grenzwerte(1) Grenzwerte(1) Grenzwerte(2) Grenzwerte(2)],...
    [1 7 7 1], 'y', 'FaceAlpha', 0.05,'EdgeColor','none');
fill([Grenzwerte(2) Grenzwerte(2) Grenzwerte(3) Grenzwerte(3)],...
    [1 7 7 1], 'b', 'FaceAlpha', 0.05,'EdgeColor','none');
fill([Grenzwerte(3) Grenzwerte(3) Grenzwerte(4) Grenzwerte(4)],...
    [1 7 7 1], 'y', 'FaceAlpha', 0.05,'EdgeColor','none');
fill([Grenzwerte(4) Grenzwerte(4) Grenzwerte(4)+seconds(30) Grenzwerte(4)+seconds(30)],...
    [1 7 7 1], 'b', 'FaceAlpha', 0.05,'EdgeColor','none');
title([Algorithm_Name ': Processed Data']);
set(gca, 'XColor', [0 0 0])
set(gca, 'YColor', [0 0 0])
hold off;
% ExCuSe -----------------------------------------------------------------------------------
% ----------------------------------------------------------------------------------------



% ----------------------------------------------------------------------------------------
% PuRe -----------------------------------------------------------------------------------
Current_Data = Plotting_Struct.PuRe.A_PupilSmooth_PuRe.Struct_Plotting;
Algorithm_Name = 'PuRe';

% Plot 1
nexttile(3);
plot(Current_Data.Pupil_Data_vorher.Datetime, Current_Data.Pupil_Data_vorher.PupilDiameterLeft);
xlim([min(Current_Data.Pupil_Data_vorher.Datetime)...
    max(Current_Data.Pupil_Data_vorher.Datetime)]);
box off; grid off;
gcaObj = gca;
gcaObj.LineWidth=1;
set(gcaObj,'TickDir','out');
gcaObj.XAxis.TickLength = [0.0150 0.0250];
gcaObj.YAxis.TickLength = [0.0150 0.0250];
set(gcaObj, 'YLim', [1 7], 'YTick', [1:2:7], 'YMinorTick','off');
set(gcaObj, 'XTick', [Current_Data.Pupil_Data_vorher.Datetime(1) Current_Data.Pupil_Data_vorher.Datetime(1)+seconds(90)...
    Current_Data.Pupil_Data_vorher.Datetime(1)+seconds(120) Current_Data.Pupil_Data_vorher.Datetime(1)+seconds(210)...
    Current_Data.Pupil_Data_vorher.Datetime(1)+seconds(240)]);
xtickformat('s')
xticklabels({'0', '90', '120', '210', '240'})
hold on;

for i = 1:2:(size(Current_Data.T,1)-1)
    fill( [ Current_Data.T.DateTime(i,1)  Current_Data.T.DateTime(i,1) Current_Data.T.DateTime(i+1,1) Current_Data.T.DateTime(i+1,1) ], [1 7 7 1], 'y', 'FaceAlpha', 0.05,'EdgeColor','none');
    fill( [ Current_Data.T.DateTime(i+1,1)  Current_Data.T.DateTime(i+1,1) Current_Data.T.DateTime(i+2,1) Current_Data.T.DateTime(i+2,1) ], [1 7 7 1], 'b', 'FaceAlpha', 0.05,'EdgeColor','none');
end
scatter(Current_Data.Pupil_onset_Table.Datetime, Current_Data.Pupil_onset_Table.PupilDiameterLeft, 10,...
    'MarkerEdgeColor',[0.4660 0.6740 0.1880], 'MarkerFaceColor',[0.4660 0.6740 0.1880], 'LineWidth', 0.1);
scatter(Current_Data.Pupil_offset_Table.Datetime, Current_Data.Pupil_offset_Table.PupilDiameterLeft, 10,...
    'MarkerEdgeColor',[0.6350 0.0780 0.1840], 'MarkerFaceColor',[0.6350 0.0780 0.1840], 'LineWidth', 0.1);
title([Algorithm_Name ': Raw Data']);
set(gca, 'XColor', [0 0 0])
set(gca, 'YColor', [0 0 0])
hold off;

% Plot 2
nexttile(6);
plot(Current_Data.Pupil_Data_without_Blink_and_Outlier.Datetime, Current_Data.Pupil_Data_without_Blink_and_Outlier.PupilDiameterLeft);
xlim([min(Current_Data.Pupil_Data_without_Blink_and_Outlier.Datetime) max(Current_Data.Pupil_Data_without_Blink_and_Outlier.Datetime)]);
box off; grid off;
gcaObj = gca;
gcaObj.LineWidth=1;
set(gcaObj,'TickDir','out');
gcaObj.XAxis.TickLength = [0.0150 0.0250];
gcaObj.YAxis.TickLength = [0.0150 0.0250];
set(gcaObj, 'YLim', [1 7], 'YTick', [1:2:7], 'YMinorTick','off');
set(gcaObj, 'XTick', [Current_Data.Pupil_Data_without_Blink_and_Outlier.Datetime(1) Current_Data.Pupil_Data_without_Blink_and_Outlier.Datetime(1)+seconds(90)...
    Current_Data.Pupil_Data_without_Blink_and_Outlier.Datetime(1)+seconds(120) Current_Data.Pupil_Data_without_Blink_and_Outlier.Datetime(1)+seconds(210)...
    Current_Data.Pupil_Data_without_Blink_and_Outlier.Datetime(1)+seconds(240)]);
xtickformat('s')
xticklabels({'0', '90', '120', '210', '240'})

hold on
Grenzwerte = splitapply(@(x) x(1), Current_Data.Pupil_Data_without_Blink_and_Outlier.Datetime,...
    findgroups(Current_Data.Pupil_Data_without_Blink_and_Outlier.SpektrumCode));

fill([Grenzwerte(1) Grenzwerte(1) Grenzwerte(2) Grenzwerte(2)],...
    [1 7 7 1], 'y', 'FaceAlpha', 0.05,'EdgeColor','none');
fill([Grenzwerte(2) Grenzwerte(2) Grenzwerte(3) Grenzwerte(3)],...
    [1 7 7 1], 'b', 'FaceAlpha', 0.05,'EdgeColor','none');
fill([Grenzwerte(3) Grenzwerte(3) Grenzwerte(4) Grenzwerte(4)],...
    [1 7 7 1], 'y', 'FaceAlpha', 0.05,'EdgeColor','none');
fill([Grenzwerte(4) Grenzwerte(4) Grenzwerte(4)+seconds(30) Grenzwerte(4)+seconds(30)],...
    [1 7 7 1], 'b', 'FaceAlpha', 0.05,'EdgeColor','none');
title([Algorithm_Name ': Processed Data']);
set(gca, 'XColor', [0 0 0])
set(gca, 'YColor', [0 0 0])
hold off;
% PuRe -----------------------------------------------------------------------------------
% ----------------------------------------------------------------------------------------



% ----------------------------------------------------------------------------------------
% PuReST -----------------------------------------------------------------------------------
Current_Data = Plotting_Struct.PuReST.A_PupilSmooth_PuReST.Struct_Plotting;
Algorithm_Name = 'PuReST';

% Plot 1
nexttile(7);
plot(Current_Data.Pupil_Data_vorher.Datetime, Current_Data.Pupil_Data_vorher.PupilDiameterLeft);
xlim([min(Current_Data.Pupil_Data_vorher.Datetime)...
    max(Current_Data.Pupil_Data_vorher.Datetime)]);
box off; grid off;
gcaObj = gca;
gcaObj.LineWidth=1;
set(gcaObj,'TickDir','out');
gcaObj.XAxis.TickLength = [0.0150 0.0250];
gcaObj.YAxis.TickLength = [0.0150 0.0250];
set(gcaObj, 'YLim', [1 7], 'YTick', [1:2:7], 'YMinorTick','off');
set(gcaObj, 'XTick', [Current_Data.Pupil_Data_vorher.Datetime(1) Current_Data.Pupil_Data_vorher.Datetime(1)+seconds(90)...
    Current_Data.Pupil_Data_vorher.Datetime(1)+seconds(120) Current_Data.Pupil_Data_vorher.Datetime(1)+seconds(210)...
    Current_Data.Pupil_Data_vorher.Datetime(1)+seconds(240)]);
xtickformat('s')
xticklabels({'0', '90', '120', '210', '240'})
hold on;

for i = 1:2:(size(Current_Data.T,1)-1)
    fill( [ Current_Data.T.DateTime(i,1)  Current_Data.T.DateTime(i,1) Current_Data.T.DateTime(i+1,1) Current_Data.T.DateTime(i+1,1) ], [1 7 7 1], 'y', 'FaceAlpha', 0.05,'EdgeColor','none');
    fill( [ Current_Data.T.DateTime(i+1,1)  Current_Data.T.DateTime(i+1,1) Current_Data.T.DateTime(i+2,1) Current_Data.T.DateTime(i+2,1) ], [1 7 7 1], 'b', 'FaceAlpha', 0.05,'EdgeColor','none');
end
scatter(Current_Data.Pupil_onset_Table.Datetime, Current_Data.Pupil_onset_Table.PupilDiameterLeft, 10,...
    'MarkerEdgeColor',[0.4660 0.6740 0.1880], 'MarkerFaceColor',[0.4660 0.6740 0.1880], 'LineWidth', 0.1);
scatter(Current_Data.Pupil_offset_Table.Datetime, Current_Data.Pupil_offset_Table.PupilDiameterLeft, 10,...
    'MarkerEdgeColor',[0.6350 0.0780 0.1840], 'MarkerFaceColor',[0.6350 0.0780 0.1840], 'LineWidth', 0.1);
title([Algorithm_Name ': Raw Data']);
set(gca, 'XColor', [0 0 0])
set(gca, 'YColor', [0 0 0])
hold off;

% Plot 2
nexttile(10);
plot(Current_Data.Pupil_Data_without_Blink_and_Outlier.Datetime, Current_Data.Pupil_Data_without_Blink_and_Outlier.PupilDiameterLeft);
xlim([min(Current_Data.Pupil_Data_without_Blink_and_Outlier.Datetime) max(Current_Data.Pupil_Data_without_Blink_and_Outlier.Datetime)]);
box off; grid off;
gcaObj = gca;
gcaObj.LineWidth=1;
set(gcaObj,'TickDir','out');
gcaObj.XAxis.TickLength = [0.0150 0.0250];
gcaObj.YAxis.TickLength = [0.0150 0.0250];
set(gcaObj, 'YLim', [1 7], 'YTick', [1:2:7], 'YMinorTick','off');
set(gcaObj, 'XTick', [Current_Data.Pupil_Data_without_Blink_and_Outlier.Datetime(1) Current_Data.Pupil_Data_without_Blink_and_Outlier.Datetime(1)+seconds(90)...
    Current_Data.Pupil_Data_without_Blink_and_Outlier.Datetime(1)+seconds(120) Current_Data.Pupil_Data_without_Blink_and_Outlier.Datetime(1)+seconds(210)...
    Current_Data.Pupil_Data_without_Blink_and_Outlier.Datetime(1)+seconds(240)]);
xtickformat('s')
xticklabels({'0', '90', '120', '210', '240'})

hold on
Grenzwerte = splitapply(@(x) x(1), Current_Data.Pupil_Data_without_Blink_and_Outlier.Datetime,...
    findgroups(Current_Data.Pupil_Data_without_Blink_and_Outlier.SpektrumCode));

fill([Grenzwerte(1) Grenzwerte(1) Grenzwerte(2) Grenzwerte(2)],...
    [1 7 7 1], 'y', 'FaceAlpha', 0.05,'EdgeColor','none');
fill([Grenzwerte(2) Grenzwerte(2) Grenzwerte(3) Grenzwerte(3)],...
    [1 7 7 1], 'b', 'FaceAlpha', 0.05,'EdgeColor','none');
fill([Grenzwerte(3) Grenzwerte(3) Grenzwerte(4) Grenzwerte(4)],...
    [1 7 7 1], 'y', 'FaceAlpha', 0.05,'EdgeColor','none');
fill([Grenzwerte(4) Grenzwerte(4) Grenzwerte(4)+seconds(30) Grenzwerte(4)+seconds(30)],...
    [1 7 7 1], 'b', 'FaceAlpha', 0.05,'EdgeColor','none');
title([Algorithm_Name ': Processed Data']);
set(gca, 'XColor', [0 0 0])
set(gca, 'YColor', [0 0 0])
hold off;
% PuReST -----------------------------------------------------------------------------------
% ----------------------------------------------------------------------------------------


% ----------------------------------------------------------------------------------------
% Starbust -----------------------------------------------------------------------------------
Current_Data = Plotting_Struct.Starbust.A_PupilSmooth_Starbust.Struct_Plotting;
Algorithm_Name = 'Starbust';

% Plot 1
nexttile(8);
plot(Current_Data.Pupil_Data_vorher.Datetime, Current_Data.Pupil_Data_vorher.PupilDiameterLeft);
xlim([min(Current_Data.Pupil_Data_vorher.Datetime)...
    max(Current_Data.Pupil_Data_vorher.Datetime)]);
box off; grid off;
gcaObj = gca;
gcaObj.LineWidth=1;
set(gcaObj,'TickDir','out');
gcaObj.XAxis.TickLength = [0.0150 0.0250];
gcaObj.YAxis.TickLength = [0.0150 0.0250];
set(gcaObj, 'YLim', [1 7], 'YTick', [1:2:7], 'YMinorTick','off');
set(gcaObj, 'XTick', [Current_Data.Pupil_Data_vorher.Datetime(1) Current_Data.Pupil_Data_vorher.Datetime(1)+seconds(90)...
    Current_Data.Pupil_Data_vorher.Datetime(1)+seconds(120) Current_Data.Pupil_Data_vorher.Datetime(1)+seconds(210)...
    Current_Data.Pupil_Data_vorher.Datetime(1)+seconds(240)]);
xtickformat('s')
xticklabels({'0', '90', '120', '210', '240'})
hold on;

for i = 1:2:(size(Current_Data.T,1)-1)
    fill( [ Current_Data.T.DateTime(i,1)  Current_Data.T.DateTime(i,1) Current_Data.T.DateTime(i+1,1) Current_Data.T.DateTime(i+1,1) ], [1 7 7 1], 'y', 'FaceAlpha', 0.05,'EdgeColor','none');
    fill( [ Current_Data.T.DateTime(i+1,1)  Current_Data.T.DateTime(i+1,1) Current_Data.T.DateTime(i+2,1) Current_Data.T.DateTime(i+2,1) ], [1 7 7 1], 'b', 'FaceAlpha', 0.05,'EdgeColor','none');
end
scatter(Current_Data.Pupil_onset_Table.Datetime, Current_Data.Pupil_onset_Table.PupilDiameterLeft, 10,...
    'MarkerEdgeColor',[0.4660 0.6740 0.1880], 'MarkerFaceColor',[0.4660 0.6740 0.1880], 'LineWidth', 0.1);
scatter(Current_Data.Pupil_offset_Table.Datetime, Current_Data.Pupil_offset_Table.PupilDiameterLeft, 10,...
    'MarkerEdgeColor',[0.6350 0.0780 0.1840], 'MarkerFaceColor',[0.6350 0.0780 0.1840], 'LineWidth', 0.1);
title([Algorithm_Name ': Raw Data']);
set(gca, 'XColor', [0 0 0])
set(gca, 'YColor', [0 0 0])
hold off;

% Plot 2
nexttile(11);
plot(Current_Data.Pupil_Data_without_Blink_and_Outlier.Datetime, Current_Data.Pupil_Data_without_Blink_and_Outlier.PupilDiameterLeft);
xlim([min(Current_Data.Pupil_Data_without_Blink_and_Outlier.Datetime) max(Current_Data.Pupil_Data_without_Blink_and_Outlier.Datetime)]);
box off; grid off;
gcaObj = gca;
gcaObj.LineWidth=1;
set(gcaObj,'TickDir','out');
gcaObj.XAxis.TickLength = [0.0150 0.0250];
gcaObj.YAxis.TickLength = [0.0150 0.0250];
set(gcaObj, 'YLim', [1 7], 'YTick', [1:2:7], 'YMinorTick','off');
set(gcaObj, 'XTick', [Current_Data.Pupil_Data_without_Blink_and_Outlier.Datetime(1) Current_Data.Pupil_Data_without_Blink_and_Outlier.Datetime(1)+seconds(90)...
    Current_Data.Pupil_Data_without_Blink_and_Outlier.Datetime(1)+seconds(120) Current_Data.Pupil_Data_without_Blink_and_Outlier.Datetime(1)+seconds(210)...
    Current_Data.Pupil_Data_without_Blink_and_Outlier.Datetime(1)+seconds(240)]);
xtickformat('s')
xticklabels({'0', '90', '120', '210', '240'})

hold on
Grenzwerte = splitapply(@(x) x(1), Current_Data.Pupil_Data_without_Blink_and_Outlier.Datetime,...
    findgroups(Current_Data.Pupil_Data_without_Blink_and_Outlier.SpektrumCode));

fill([Grenzwerte(1) Grenzwerte(1) Grenzwerte(2) Grenzwerte(2)],...
    [1 7 7 1], 'y', 'FaceAlpha', 0.05,'EdgeColor','none');
fill([Grenzwerte(2) Grenzwerte(2) Grenzwerte(3) Grenzwerte(3)],...
    [1 7 7 1], 'b', 'FaceAlpha', 0.05,'EdgeColor','none');
fill([Grenzwerte(3) Grenzwerte(3) Grenzwerte(4) Grenzwerte(4)],...
    [1 7 7 1], 'y', 'FaceAlpha', 0.05,'EdgeColor','none');
fill([Grenzwerte(4) Grenzwerte(4) Grenzwerte(4)+seconds(30) Grenzwerte(4)+seconds(30)],...
    [1 7 7 1], 'b', 'FaceAlpha', 0.05,'EdgeColor','none');
title([Algorithm_Name ': Processed Data']);
set(gca, 'XColor', [0 0 0])
set(gca, 'YColor', [0 0 0])
hold off;
% Starbust -----------------------------------------------------------------------------------
% ----------------------------------------------------------------------------------------


% ----------------------------------------------------------------------------------------
% Swirski2D -----------------------------------------------------------------------------------
Current_Data = Plotting_Struct.Swirski2D.A_PupilSmooth_Swirski.Struct_Plotting;
Algorithm_Name = 'Swirski2D';

% Plot 1
nexttile(9);
plot(Current_Data.Pupil_Data_vorher.Datetime, Current_Data.Pupil_Data_vorher.PupilDiameterLeft);
xlim([min(Current_Data.Pupil_Data_vorher.Datetime)...
    max(Current_Data.Pupil_Data_vorher.Datetime)]);
box off; grid off;
gcaObj = gca;
gcaObj.LineWidth=1;
set(gcaObj,'TickDir','out');
gcaObj.XAxis.TickLength = [0.0150 0.0250];
gcaObj.YAxis.TickLength = [0.0150 0.0250];
set(gcaObj, 'YLim', [1 7], 'YTick', [1:2:7], 'YMinorTick','off');
set(gcaObj, 'XTick', [Current_Data.Pupil_Data_vorher.Datetime(1) Current_Data.Pupil_Data_vorher.Datetime(1)+seconds(90)...
    Current_Data.Pupil_Data_vorher.Datetime(1)+seconds(120) Current_Data.Pupil_Data_vorher.Datetime(1)+seconds(210)...
    Current_Data.Pupil_Data_vorher.Datetime(1)+seconds(240)]);
xtickformat('s')
xticklabels({'0', '90', '120', '210', '240'})
hold on;

for i = 1:2:(size(Current_Data.T,1)-1)
    fill( [ Current_Data.T.DateTime(i,1)  Current_Data.T.DateTime(i,1) Current_Data.T.DateTime(i+1,1) Current_Data.T.DateTime(i+1,1) ], [1 7 7 1], 'y', 'FaceAlpha', 0.05,'EdgeColor','none');
    fill( [ Current_Data.T.DateTime(i+1,1)  Current_Data.T.DateTime(i+1,1) Current_Data.T.DateTime(i+2,1) Current_Data.T.DateTime(i+2,1) ], [1 7 7 1], 'b', 'FaceAlpha', 0.05,'EdgeColor','none');
end
scatter(Current_Data.Pupil_onset_Table.Datetime, Current_Data.Pupil_onset_Table.PupilDiameterLeft, 10,...
    'MarkerEdgeColor',[0.4660 0.6740 0.1880], 'MarkerFaceColor',[0.4660 0.6740 0.1880], 'LineWidth', 0.1);
scatter(Current_Data.Pupil_offset_Table.Datetime, Current_Data.Pupil_offset_Table.PupilDiameterLeft, 10,...
    'MarkerEdgeColor',[0.6350 0.0780 0.1840], 'MarkerFaceColor',[0.6350 0.0780 0.1840], 'LineWidth', 0.1);
title([Algorithm_Name ': Raw Data']);
set(gca, 'XColor', [0 0 0])
set(gca, 'YColor', [0 0 0])
hold off;

% Plot 2
nexttile(12);
plot(Current_Data.Pupil_Data_without_Blink_and_Outlier.Datetime, Current_Data.Pupil_Data_without_Blink_and_Outlier.PupilDiameterLeft);
xlim([min(Current_Data.Pupil_Data_without_Blink_and_Outlier.Datetime) max(Current_Data.Pupil_Data_without_Blink_and_Outlier.Datetime)]);
box off; grid off;
gcaObj = gca;
gcaObj.LineWidth=1;
set(gcaObj,'TickDir','out');
gcaObj.XAxis.TickLength = [0.0150 0.0250];
gcaObj.YAxis.TickLength = [0.0150 0.0250];
set(gcaObj, 'YLim', [1 7], 'YTick', [1:2:7], 'YMinorTick','off');
set(gcaObj, 'XTick', [Current_Data.Pupil_Data_without_Blink_and_Outlier.Datetime(1) Current_Data.Pupil_Data_without_Blink_and_Outlier.Datetime(1)+seconds(90)...
    Current_Data.Pupil_Data_without_Blink_and_Outlier.Datetime(1)+seconds(120) Current_Data.Pupil_Data_without_Blink_and_Outlier.Datetime(1)+seconds(210)...
    Current_Data.Pupil_Data_without_Blink_and_Outlier.Datetime(1)+seconds(240)]);
xtickformat('s')
xticklabels({'0', '90', '120', '210', '240'})

hold on
Grenzwerte = splitapply(@(x) x(1), Current_Data.Pupil_Data_without_Blink_and_Outlier.Datetime,...
    findgroups(Current_Data.Pupil_Data_without_Blink_and_Outlier.SpektrumCode));

fill([Grenzwerte(1) Grenzwerte(1) Grenzwerte(2) Grenzwerte(2)],...
    [1 7 7 1], 'y', 'FaceAlpha', 0.05,'EdgeColor','none');
fill([Grenzwerte(2) Grenzwerte(2) Grenzwerte(3) Grenzwerte(3)],...
    [1 7 7 1], 'b', 'FaceAlpha', 0.05,'EdgeColor','none');
fill([Grenzwerte(3) Grenzwerte(3) Grenzwerte(4) Grenzwerte(4)],...
    [1 7 7 1], 'y', 'FaceAlpha', 0.05,'EdgeColor','none');
fill([Grenzwerte(4) Grenzwerte(4) Grenzwerte(4)+seconds(30) Grenzwerte(4)+seconds(30)],...
    [1 7 7 1], 'b', 'FaceAlpha', 0.05,'EdgeColor','none');
title([Algorithm_Name ': Processed Data']);
set(gca, 'XColor', [0 0 0])
set(gca, 'YColor', [0 0 0])
hold off;
% Swirski2D -----------------------------------------------------------------------------------
% ----------------------------------------------------------------------------------------

hx = xlabel(t,'Time in seconds', 'FontSize', 8, 'FontName', 'Charter', 'Color', 'k');
hy = ylabel(t,'Pupil diameter in mm', 'FontSize', 8, 'FontName', 'Charter', 'Color', 'k');

% Export plot as pdf ------------------------------------
FontName = 'Charter';
fig.Units = 'centimeters';
fig.Renderer = 'painters';
fig.PaperPositionMode = 'manual';
set(findall(gcf,'-property','FontSize'),'FontSize',8)
set(findall(gcf,'-property','FontName'),'FontName', FontName)
% Scatter size - LineWidth
set(findobj(gcf, 'Linewidth', 0.1), 'Linewidth',0.01)
% Scatter size - SizeData (MarkerSize)
set(findobj(gcf, 'SizeData', 10), 'SizeData', 5)
% LineWidth Axes
set(findobj(gcf, 'Linewidth', 1), 'Linewidth', 0.5)
% LineWidth Lineplots
set(findobj(gcf, 'Linewidth', 2), 'Linewidth', 0.5)
set(hx, 'FontSize', 8, 'FontName', FontName)
set(hy, 'FontSize', 8, 'FontName', FontName)
set(gcf, 'Position', [0.1, 11, 15.8, 9]); % [PositionDesk, PositionDesk, Width, Height]
exportgraphics(gcf, '02_Exported_Plots/TimePlot_Algos_Trial_1.pdf','ContentType','vector')
% ---------------------------------------------------------------

%% Plotting B): Barplot from the invalid data rate in percent for each algorithm
clc; clear; close all;
load('00_Data/Pupil_Data')

Data = PupilSmooth_Data.Invalid_Data_Performance;

% Step 1: Calculate the mean for each algorithm
Data.Algorithm = categorical(Data.Algorithm);
Data.Trial = categorical(Data.Trial);
Data.Spektrumsbezeichnung = categorical(Data.Spektrumsbezeichnung);

MeanValues_Table = groupsummary(Data,{'Spektrumsbezeichnung','Algorithm'},{'mean', 'std'},'Invalid_Data_Percent');
MeanValues_Table = removevars(MeanValues_Table, 'GroupCount');
MeanValues_Table.Properties.VariableNames = {'Spectrum', 'Algorithm', 'Mean', 'SD'};

% Vier Bars für jeden Algos
Else = MeanValues_Table(MeanValues_Table.Algorithm == 'Else', :);
ExCuSe = MeanValues_Table(MeanValues_Table.Algorithm == 'ExCuSe', :);
PuRe = MeanValues_Table(MeanValues_Table.Algorithm == 'PuRe', :);
PuReST = MeanValues_Table(MeanValues_Table.Algorithm == 'PuReST', :);
Starbust = MeanValues_Table(MeanValues_Table.Algorithm == 'Starbust', :);
Swirski2D = MeanValues_Table(MeanValues_Table.Algorithm == 'Swirski2D', :);

Mean_Array = [Else.Mean(3), Else.Mean(1), Else.Mean(4), Else.Mean(2);...
    ExCuSe.Mean(3), ExCuSe.Mean(1), ExCuSe.Mean(4), ExCuSe.Mean(2);...
    PuRe.Mean(3), PuRe.Mean(1), PuRe.Mean(4), PuRe.Mean(2);...
    PuReST.Mean(3), PuReST.Mean(1), PuReST.Mean(4), PuReST.Mean(2);...
    Starbust.Mean(3), Starbust.Mean(1), Starbust.Mean(4), Starbust.Mean(2);...
    Swirski2D.Mean(3), Swirski2D.Mean(1), Swirski2D.Mean(4), Swirski2D.Mean(2)];

SD_Array = [Else.SD(3), Else.SD(1), Else.SD(4), Else.SD(2);...
    ExCuSe.SD(3), ExCuSe.SD(1), ExCuSe.SD(4), ExCuSe.SD(2);...
    PuRe.SD(3), PuRe.SD(1), PuRe.SD(4), PuRe.SD(2);...
    PuReST.SD(3), PuReST.SD(1), PuReST.SD(4), PuReST.SD(2);...
    Starbust.SD(3), Starbust.SD(1), Starbust.SD(4), Starbust.SD(2);...
    Swirski2D.SD(3), Swirski2D.SD(1), Swirski2D.SD(4), Swirski2D.SD(2)];

% Presets for plotting
fig = figure;
set(gcf, 'Position', [0.1, 11, 7, 4]);
t = tiledlayout('flow', 'Padding','none');

nexttile;
x = [1 2 3 4 5 6];
b = bar(x, Mean_Array, 0.8); hold on;
xBar=cell2mat(get(b,'XData')).' + [b.XOffset];
hEB=errorbar(xBar, Mean_Array, SD_Array,'k.');
set(gca, 'YLim', [0, 100], 'YTick',0:20:100, 'FontSize', 12);
set(gca, 'XLim', [0.5, 6.5]);
set(gca, 'XColor', [0 0 0])
set(gca, 'YColor', [0 0 0])

% Colors
b(1).FaceColor = [0.670, 0.850, 0.913]; % Reference 1 Spectrum
b(2).FaceColor = [0.172, 0.482, 0.713]; % 450 nm Spectrum
b(3).FaceColor = [0.992, 0.682, 0.380]; % Reference 2 Spectrum
b(4).FaceColor = [0.843, 0.098, 0.109]; % 630 nm Spectrum

set(gca,'XTickLabel',cellstr(MeanValues_Table.Algorithm))
set(gca,'XminorTick','off')
hold off; box on; grid on;
hx = ylabel('Invalid data rate in %', 'FontSize', 12, 'Color', 'k');
hy = xlabel('Pupil detection algorithm', 'FontSize', 12, 'Color', 'k');

% Export plot as pdf ------------------------------------
% FontName = 'Charter';
% fig.Units = 'centimeters';
% fig.Renderer = 'painters';
% fig.PaperPositionMode = 'manual';
% set(findall(gcf,'-property','FontSize'),'FontSize',8)
% set(findall(gcf,'-property','FontName'),'FontName', FontName)
% set(findall(gcf,'-property','Linewidth'),'Linewidth', 0.5)
% set(hx, 'FontSize', 8, 'FontName', FontName)
% set(hy, 'FontSize', 8, 'FontName', FontName)
% set(gcf, 'Position', [0.1, 11, 11, 6]); % [PositionDesk, PositionDesk, Width, Height]
% exportgraphics(gcf, '02_Exported_Plots/Barplot_Invalid_Data.pdf','ContentType','vector')
% ---------------------------------------------------------------

%% Plotting C): Boxplot of the pupil diamater (mean of last 5 seconds) for each wavelength and algorithm. Each wavelength has its own facet.

% Export width: 88 mm (1 coloumn)

% -----------------------------------------------------------------------
% Comment: The detection results from Starbust and Swirski2D will not be
% analysed as they have an invalid data rate higher than 20%
% -----------------------------------------------------------------------

clc; clear; close all;
load('00_Data/Pupil_Data')
Mean_WindowSec = 5;
From_WhichTime_Refr = 85;
From_WhichTime_Stim = 25;

AlgoName = 'ElSe';
Current_Dataset = Long_PupilSmooth_ElSe;
Vola_Data = get_Pupil_Data_at(Current_Dataset, Mean_WindowSec, From_WhichTime_Stim);
ElSe_Mean_Data = [get_Pupil_Data_at(Current_Dataset, Mean_WindowSec, From_WhichTime_Refr);...
    Vola_Data(Vola_Data.Spektrumbezeichnung ~= "Referenz_1" & Vola_Data.Spektrumbezeichnung ~= "Referenz_2", :)];
ElSe_Mean_Data.AlgoName(:) = cellstr(AlgoName);

AlgoName = 'ExCuSe';
Current_Dataset = Long_PupilSmooth_ExCuSe;
Vola_Data = get_Pupil_Data_at(Current_Dataset, Mean_WindowSec, From_WhichTime_Stim);
ExCuSe_Mean_Data = [get_Pupil_Data_at(Current_Dataset, Mean_WindowSec, From_WhichTime_Refr);...
    Vola_Data(Vola_Data.Spektrumbezeichnung ~= "Referenz_1" & Vola_Data.Spektrumbezeichnung ~= "Referenz_2", :)];
ExCuSe_Mean_Data.AlgoName(:) = cellstr(AlgoName);

AlgoName = 'PuRe';
Current_Dataset = Long_PupilSmooth_PuRe;
Vola_Data = get_Pupil_Data_at(Current_Dataset, Mean_WindowSec, From_WhichTime_Stim);
PuRe_Mean_Data = [get_Pupil_Data_at(Current_Dataset, Mean_WindowSec, From_WhichTime_Refr);...
    Vola_Data(Vola_Data.Spektrumbezeichnung ~= "Referenz_1" & Vola_Data.Spektrumbezeichnung ~= "Referenz_2", :)];
PuRe_Mean_Data.AlgoName(:) = cellstr(AlgoName);

AlgoName = 'PuReST';
Current_Dataset = Long_PupilSmooth_PuReST;
Vola_Data = get_Pupil_Data_at(Current_Dataset, Mean_WindowSec, From_WhichTime_Stim);
PuReST_Mean_Data = [get_Pupil_Data_at(Current_Dataset, Mean_WindowSec, From_WhichTime_Refr);...
    Vola_Data(Vola_Data.Spektrumbezeichnung ~= "Referenz_1" & Vola_Data.Spektrumbezeichnung ~= "Referenz_2", :)];
PuReST_Mean_Data.AlgoName(:) = cellstr(AlgoName);

Long_Table_Mean = [ElSe_Mean_Data; ExCuSe_Mean_Data; PuRe_Mean_Data; PuReST_Mean_Data];

% Plotting: One Boxplot for each stimulus -> 2 x 2 grid
fig = figure;
set(gcf, 'Position', [0.1, 11, 7, 4]);
t = tiledlayout(2, 2, 'TileSpacing', 'compact', 'Padding','none');

% Reference 1
nexttile;
Current_Data = Long_Table_Mean(Long_Table_Mean.Spektrumbezeichnung == "Referenz_1", :);
Current_Data.AlgoName = categorical(Current_Data.AlgoName);
boxplot(Current_Data.PupilDiameterLeft, Current_Data.AlgoName); hold on;
f1 = scatter(findgroups(Current_Data.AlgoName), Current_Data.PupilDiameterLeft,...
    15, 'k','filled', 'jitter','on', 'jitterAmount',0.1); hold off; box off; grid off;
f1.MarkerFaceAlpha = 0.3;
title('Reference 1: 5500 K', 'FontSize', 12, 'FontName', 'Charter');
set(gca, 'XColor', [0 0 0]); set(gca, 'YColor', [0 0 0]);
set(gca, 'YLim', [2.5, 2.95], 'YTick',2.5:0.15:2.95, 'FontSize', 12);
set(gca, 'XLim', [0.5, 4.5], 'FontSize', 12);
set(gca,'XminorTick','off')

% Reference 2
nexttile;
Current_Data = Long_Table_Mean(Long_Table_Mean.Spektrumbezeichnung == "Referenz_2", :);
Current_Data.AlgoName = categorical(Current_Data.AlgoName);
boxplot(Current_Data.PupilDiameterLeft, Current_Data.AlgoName); hold on;
f1 = scatter(findgroups(Current_Data.AlgoName), Current_Data.PupilDiameterLeft,...
    15, 'k','filled', 'jitter','on', 'jitterAmount',0.1); hold off; box off; grid off;
f1.MarkerFaceAlpha = 0.3;
title('Reference 2: 5500 K', 'FontSize', 12, 'FontName', 'Charter');
set(gca, 'XColor', [0 0 0]); set(gca, 'YColor', [0 0 0]);
set(gca, 'YLim', [2.5, 2.95], 'YTick',2.5:0.15:2.95, 'FontSize', 12);
set(gca, 'XLim', [0.5, 4.5], 'FontSize', 12);
set(gca,'XminorTick','off')

% Stimulus: 450 nm
nexttile;
Current_Data = Long_Table_Mean(Long_Table_Mean.Spektrumbezeichnung == "450_nm", :);
Current_Data.AlgoName = categorical(Current_Data.AlgoName);
boxplot(Current_Data.PupilDiameterLeft, Current_Data.AlgoName); hold on;
f1 = scatter(findgroups(Current_Data.AlgoName), Current_Data.PupilDiameterLeft,...
    15, 'k','filled', 'jitter','on', 'jitterAmount',0.1); hold off; box off; grid off;
f1.MarkerFaceAlpha = 0.3;
title('Stimulus: \lambda_{Peak} = 450 nm', 'FontSize', 12, 'FontName', 'Charter');
set(gca, 'XColor', [0 0 0]); set(gca, 'YColor', [0 0 0]);
set(gca, 'YLim', [2, 2.45], 'YTick',2:0.15:2.45, 'FontSize', 12);
set(gca, 'XLim', [0.5, 4.5], 'FontSize', 12);
set(gca,'XminorTick','off')

% Stimulus: 630 nm
nexttile;
Current_Data = Long_Table_Mean(Long_Table_Mean.Spektrumbezeichnung == "630_nm", :);
Current_Data.AlgoName = categorical(Current_Data.AlgoName);
boxplot(Current_Data.PupilDiameterLeft, Current_Data.AlgoName); hold on;
f1 = scatter(findgroups(Current_Data.AlgoName), Current_Data.PupilDiameterLeft,...
    15, 'k','filled', 'jitter','on', 'jitterAmount',0.1); hold off; box off; grid off;
f1.MarkerFaceAlpha = 0.3;
title('Stimulus: \lambda_{Peak} = 630 nm', 'FontSize', 12, 'FontName', 'Charter');
set(gca, 'XColor', [0 0 0]); set(gca, 'YColor', [0 0 0]);
set(gca, 'YLim', [3, 3.45], 'YTick',3:0.15:3.45, 'FontSize', 12);
set(gca, 'XLim', [0.5, 4.5], 'FontSize', 12);
set(gca,'XminorTick','off')

hx = xlabel(t,'Pupil detection algorithm', 'FontSize', 12, 'FontName', 'Charter', 'Color', 'k');
hy = ylabel(t,'Pupil diameter in mm', 'FontSize', 12, 'FontName', 'Charter', 'Color', 'k');

% Export plot as pdf ------------------------------------
FontName = 'Charter';
FontSize = 8;
fig.Units = 'centimeters';
fig.Renderer = 'painters';
fig.PaperPositionMode = 'manual';
set(findall(gcf,'-property','FontSize'),'FontSize', FontSize)
set(hx, 'FontSize', FontSize, 'FontName', FontName)
set(hy, 'FontSize', FontSize, 'FontName', FontName)
set(findall(gcf,'-property','FontName'),'FontName', FontName)
set(findobj(gcf, 'SizeData', 15), 'SizeData', 5) % Scatter size - SizeData (MarkerSize)
set(findobj(gcf, 'Linewidth', 1), 'Linewidth', 0.5) % LineWidth Axes
set(findobj(gcf, 'Linewidth', 2), 'Linewidth', 0.5) % LineWidth Lineplots
set(gcf, 'Position', [0.1, 11, 8.8, 6]); % [PositionDesk, PositionDesk, Width, Height]
% exportgraphics(gcf, '02_Exported_Plots/Boxplot_PD.pdf','ContentType','vector')
% ---------------------------------------------------------------

% Additonal Information:
% Here we calculated for each Trial the range of pupil diameter difference
% between the pupil detection algorithms. Ideally, the range should be
% zero, as it is the dataset. Table_Ranges gives the PupilDiameter Range
% for each trial. The mean values cannot be compared directly to each other together,
% as the mean can be different due to the pupil fluctiation.
Table_Ranges = groupsummary(Long_Table_Mean, {'Probandencode', 'Spektrumbezeichnung'},...
    {'mean', 'std', 'var', 'range'}, 'PupilDiameterLeft');

Table_Mean_Deviation =  groupsummary(Long_Table_Mean, {'AlgoName', 'Spektrumbezeichnung'},...
    {'mean', 'std', 'var', 'range'}, 'PupilDiameterLeft');

% Mean Deviation ElSe and ExCuSe (450 nm) = abs(2.2518 - 2.2524) -> 6.0000e-04 mm
% Mean Deviation ElSe and ExCuSe (630 nm) = abs(3.2517 - 3.2476) -> 0.0041 mm
% Mean Deviation ElSe and ExCuSe (Ref 1) = abs(2.7296  - 2.7298) -> 2.0000e-04 mm
% Mean Deviation ElSe and ExCuSe (Ref 2) = abs(2.6592  - 2.6597) -> 5.0000e-04 mm

% Mean Deviation ElSe and ExCuSe (450 nm) = abs(2.2994  - 2.3055) -> 6.0000e-04 mm
% Mean Deviation ElSe and ExCuSe (630 nm) = abs(3.292 - 3.2971) -> 0.0041 mm
% Mean Deviation ElSe and ExCuSe (Ref 1) = abs(2.7784  - 2.7851) -> 2.0000e-04 mm
% Mean Deviation ElSe and ExCuSe (Ref 2) = abs(2.7094  - 2.7155) -> 5.0000e-04 mm

% Caluclating the mean of the pupil diameter range between the gives
% Mean 0.05383 \pm SD 0.0043, meaning the pupil diameter can be tracked in such a
% accuracy range, depending on the used algorithm.
Summary = table(mean(Table_Ranges.range_PupilDiameterLeft), std(Table_Ranges.range_PupilDiameterLeft),...
    'VariableNames', {'Mean' 'SD'}); disp(' '); disp(Summary)

%% Plotting B) & C):
% B): Barplot from the invalid data rate in percent for each algorithm
% C): Boxplot of the pupil diamater (mean of last 5 seconds) for each wavelength and algorithm. Each wavelength has its own facet.

clc; clear; close all;
load('00_Data/Pupil_Data')

% Plotting: One Boxplot for each stimulus -> 2 x 2 grid
fig = figure;
set(gcf, 'Position', [0.1, 11, 7, 4]);
t = tiledlayout(3, 2, 'TileSpacing', 'compact', 'Padding','tight');

% Plot B) ---------------------------------------------------------------
Data = PupilSmooth_Data.Invalid_Data_Performance;

% Step 1: Calculate the mean for each algorithm
Data.Algorithm = categorical(Data.Algorithm);
Data.Trial = categorical(Data.Trial);
Data.Spektrumsbezeichnung = categorical(Data.Spektrumsbezeichnung);

MeanValues_Table = groupsummary(Data,{'Spektrumsbezeichnung','Algorithm'},{'mean', 'std'},'Invalid_Data_Percent');
MeanValues_Table = removevars(MeanValues_Table, 'GroupCount');
MeanValues_Table.Properties.VariableNames = {'Spectrum', 'Algorithm', 'Mean', 'SD'};

% Four bars for each algo
Else = MeanValues_Table(MeanValues_Table.Algorithm == 'Else', :);
ExCuSe = MeanValues_Table(MeanValues_Table.Algorithm == 'ExCuSe', :);
PuRe = MeanValues_Table(MeanValues_Table.Algorithm == 'PuRe', :);
PuReST = MeanValues_Table(MeanValues_Table.Algorithm == 'PuReST', :);
Starbust = MeanValues_Table(MeanValues_Table.Algorithm == 'Starbust', :);
Swirski2D = MeanValues_Table(MeanValues_Table.Algorithm == 'Swirski2D', :);

Mean_Array = [Else.Mean(3), Else.Mean(1), Else.Mean(4), Else.Mean(2);...
    ExCuSe.Mean(3), ExCuSe.Mean(1), ExCuSe.Mean(4), ExCuSe.Mean(2);...
    PuRe.Mean(3), PuRe.Mean(1), PuRe.Mean(4), PuRe.Mean(2);...
    PuReST.Mean(3), PuReST.Mean(1), PuReST.Mean(4), PuReST.Mean(2);...
    Starbust.Mean(3), Starbust.Mean(1), Starbust.Mean(4), Starbust.Mean(2);...
    Swirski2D.Mean(3), Swirski2D.Mean(1), Swirski2D.Mean(4), Swirski2D.Mean(2)];

SD_Array = [Else.SD(3), Else.SD(1), Else.SD(4), Else.SD(2);...
    ExCuSe.SD(3), ExCuSe.SD(1), ExCuSe.SD(4), ExCuSe.SD(2);...
    PuRe.SD(3), PuRe.SD(1), PuRe.SD(4), PuRe.SD(2);...
    PuReST.SD(3), PuReST.SD(1), PuReST.SD(4), PuReST.SD(2);...
    Starbust.SD(3), Starbust.SD(1), Starbust.SD(4), Starbust.SD(2);...
    Swirski2D.SD(3), Swirski2D.SD(1), Swirski2D.SD(4), Swirski2D.SD(2)];

nexttile(1, [1 2]);
x = [1 2 3 4 5 6];
b = bar(x, Mean_Array, 0.8); hold on;
xBar=cell2mat(get(b,'XData')).' + [b.XOffset];
hEB=errorbar(xBar, Mean_Array, SD_Array,'k.');
set(gca, 'YLim', [0, 100], 'YTick',0:20:100, 'FontSize', 12);
set(gca, 'XLim', [0.5, 6.5]);
set(gca, 'XColor', [0 0 0])
set(gca, 'YColor', [0 0 0])

% Colors
b(1).FaceColor = [0.670, 0.850, 0.913]; % Reference 1 Spectrum
b(2).FaceColor = [0.172, 0.482, 0.713]; % 450 nm Spectrum
b(3).FaceColor = [0.992, 0.682, 0.380]; % Reference 2 Spectrum
b(4).FaceColor = [0.843, 0.098, 0.109]; % 630 nm Spectrum

set(gca,'XTickLabel',cellstr(MeanValues_Table.Algorithm))
set(gca,'XminorTick','off')
hold off; box on; grid on;
hy_1 = ylabel('Invalid data rate in %', 'FontSize', 12, 'Color', 'k');
%hx_1 = xlabel('Pupil detection algorithm', 'FontSize', 12, 'Color', 'k');
% ------------------------------------------------------------------------


% Plot C) ---------------------------------------------------------------
Mean_WindowSec = 5;
From_WhichTime_Refr = 85;
From_WhichTime_Stim = 25;

AlgoName = 'ElSe';
Current_Dataset = Long_PupilSmooth_ElSe;
Vola_Data = get_Pupil_Data_at(Current_Dataset, Mean_WindowSec, From_WhichTime_Stim);
ElSe_Mean_Data = [get_Pupil_Data_at(Current_Dataset, Mean_WindowSec, From_WhichTime_Refr);...
    Vola_Data(Vola_Data.Spektrumbezeichnung ~= "Referenz_1" & Vola_Data.Spektrumbezeichnung ~= "Referenz_2", :)];
ElSe_Mean_Data.AlgoName(:) = cellstr(AlgoName);

AlgoName = 'ExCuSe';
Current_Dataset = Long_PupilSmooth_ExCuSe;
Vola_Data = get_Pupil_Data_at(Current_Dataset, Mean_WindowSec, From_WhichTime_Stim);
ExCuSe_Mean_Data = [get_Pupil_Data_at(Current_Dataset, Mean_WindowSec, From_WhichTime_Refr);...
    Vola_Data(Vola_Data.Spektrumbezeichnung ~= "Referenz_1" & Vola_Data.Spektrumbezeichnung ~= "Referenz_2", :)];
ExCuSe_Mean_Data.AlgoName(:) = cellstr(AlgoName);

AlgoName = 'PuRe';
Current_Dataset = Long_PupilSmooth_PuRe;
Vola_Data = get_Pupil_Data_at(Current_Dataset, Mean_WindowSec, From_WhichTime_Stim);
PuRe_Mean_Data = [get_Pupil_Data_at(Current_Dataset, Mean_WindowSec, From_WhichTime_Refr);...
    Vola_Data(Vola_Data.Spektrumbezeichnung ~= "Referenz_1" & Vola_Data.Spektrumbezeichnung ~= "Referenz_2", :)];
PuRe_Mean_Data.AlgoName(:) = cellstr(AlgoName);

AlgoName = 'PuReST';
Current_Dataset = Long_PupilSmooth_PuReST;
Vola_Data = get_Pupil_Data_at(Current_Dataset, Mean_WindowSec, From_WhichTime_Stim);
PuReST_Mean_Data = [get_Pupil_Data_at(Current_Dataset, Mean_WindowSec, From_WhichTime_Refr);...
    Vola_Data(Vola_Data.Spektrumbezeichnung ~= "Referenz_1" & Vola_Data.Spektrumbezeichnung ~= "Referenz_2", :)];
PuReST_Mean_Data.AlgoName(:) = cellstr(AlgoName);

Long_Table_Mean = [ElSe_Mean_Data; ExCuSe_Mean_Data; PuRe_Mean_Data; PuReST_Mean_Data];

% Reference 1
nexttile;
Current_Data = Long_Table_Mean(Long_Table_Mean.Spektrumbezeichnung == "Referenz_1", :);
Current_Data.AlgoName = categorical(Current_Data.AlgoName);
boxplot(Current_Data.PupilDiameterLeft, Current_Data.AlgoName); hold on;
f1 = scatter(findgroups(Current_Data.AlgoName), Current_Data.PupilDiameterLeft,...
    15, 'k','filled', 'jitter','on', 'jitterAmount',0.1); hold off; box off; grid off;
f1.MarkerFaceAlpha = 0.3;
title('Reference 1: 5500 K', 'FontSize', 12, 'FontName', 'Charter');
set(gca, 'XColor', [0 0 0]); set(gca, 'YColor', [0 0 0]);
set(gca, 'YLim', [2.5, 3], 'YTick',2.5:0.1:3, 'FontSize', 12);
set(gca, 'XLim', [0.5, 4.5], 'FontSize', 12);
set(gca,'XminorTick','off')

% Reference 2
nexttile;
Current_Data = Long_Table_Mean(Long_Table_Mean.Spektrumbezeichnung == "Referenz_2", :);
Current_Data.AlgoName = categorical(Current_Data.AlgoName);
boxplot(Current_Data.PupilDiameterLeft, Current_Data.AlgoName); hold on;
f1 = scatter(findgroups(Current_Data.AlgoName), Current_Data.PupilDiameterLeft,...
    15, 'k','filled', 'jitter','on', 'jitterAmount',0.1); hold off; box off; grid off;
f1.MarkerFaceAlpha = 0.3;
title('Reference 2: 5500 K', 'FontSize', 12, 'FontName', 'Charter');
set(gca, 'XColor', [0 0 0]); set(gca, 'YColor', [0 0 0]);
set(gca, 'YLim', [2.5, 3], 'YTick',2.5:0.1:3, 'FontSize', 12);
set(gca, 'XLim', [0.5, 4.5], 'FontSize', 12);
set(gca,'XminorTick','off')

% Stimulus: 450 nm
nexttile;
Current_Data = Long_Table_Mean(Long_Table_Mean.Spektrumbezeichnung == "450_nm", :);
Current_Data.AlgoName = categorical(Current_Data.AlgoName);
boxplot(Current_Data.PupilDiameterLeft, Current_Data.AlgoName); hold on;
f1 = scatter(findgroups(Current_Data.AlgoName), Current_Data.PupilDiameterLeft,...
    15, 'k','filled', 'jitter','on', 'jitterAmount',0.1); hold off; box off; grid off;
f1.MarkerFaceAlpha = 0.3;
title('Stimulus: \lambda_{Peak} = 450 nm', 'FontSize', 12, 'FontName', 'Charter');
set(gca, 'XColor', [0 0 0]); set(gca, 'YColor', [0 0 0]);
set(gca, 'YLim', [2, 2.5], 'YTick',2:0.1:2.5, 'FontSize', 12);
set(gca, 'XLim', [0.5, 4.5], 'FontSize', 12);
set(gca,'XminorTick','off')
hy_2 = ylabel('Pupil diameter in mm', 'FontSize', 12, 'FontName', 'Charter', 'Color', 'k');

% Stimulus: 630 nm
nexttile;
Current_Data = Long_Table_Mean(Long_Table_Mean.Spektrumbezeichnung == "630_nm", :);
Current_Data.AlgoName = categorical(Current_Data.AlgoName);
boxplot(Current_Data.PupilDiameterLeft, Current_Data.AlgoName); hold on;
f1 = scatter(findgroups(Current_Data.AlgoName), Current_Data.PupilDiameterLeft,...
    15, 'k','filled', 'jitter','on', 'jitterAmount',0.1); hold off; box off; grid off;
f1.MarkerFaceAlpha = 0.3;
title('Stimulus: \lambda_{Peak} = 630 nm', 'FontSize', 12, 'FontName', 'Charter');
set(gca, 'XColor', [0 0 0]); set(gca, 'YColor', [0 0 0]);
set(gca, 'YLim', [3, 3.5], 'YTick',3:0.1:3.5, 'FontSize', 12);
set(gca, 'XLim', [0.5, 4.5], 'FontSize', 12);
set(gca,'XminorTick','off')
% -----------------------------------------------------------------------------

hx_2 = xlabel(t, 'Pupil detection algorithm', 'FontSize', 12, 'FontName', 'Charter', 'Color', 'k');

% Export plot as pdf ------------------------------------
FontName = 'Charter';
FontSize = 8;
fig.Units = 'centimeters';
fig.Renderer = 'painters';
fig.PaperPositionMode = 'manual';
set(findall(gcf,'-property','FontSize'),'FontSize', FontSize)
%set(hx_1, 'FontSize', FontSize, 'FontName', FontName)
set(hy_1, 'FontSize', FontSize, 'FontName', FontName)
set(hx_2, 'FontSize', FontSize, 'FontName', FontName)
set(hy_2, 'FontSize', FontSize, 'FontName', FontName)
set(findall(gcf,'-property','FontName'),'FontName', FontName)
set(findobj(gcf, 'SizeData', 15), 'SizeData', 5) % Scatter size - SizeData (MarkerSize)
set(findobj(gcf, 'Linewidth', 1), 'Linewidth', 0.75) % LineWidth Axes
set(findobj(gcf, 'Linewidth', 2), 'Linewidth', 0.75) % LineWidth Lineplots
set(gcf, 'Position', [0.1, 11, 8.6, 12]); % [PositionDesk, PositionDesk, Width, Height]
exportgraphics(gcf, '02_Exported_Plots/Boxplot_Bar_PD.pdf','ContentType','vector')
% ---------------------------------------------------------------



