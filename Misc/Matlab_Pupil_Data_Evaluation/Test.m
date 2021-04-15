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








