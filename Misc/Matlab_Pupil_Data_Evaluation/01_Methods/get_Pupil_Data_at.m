% Achtung hier wird der linke Pupillendurchmesser zurückgegeben
function [Mittelwerte] = get_Pupil_Data_at(Long_PupilSmooth_Quasi, Mittelwerbereich_Sekunden, From_Point_Sekunden)
    
    Aktueller_Datensatz = timetable2table(Long_PupilSmooth_Quasi);
    Aktueller_Datensatz = removevars(Aktueller_Datensatz,'Datetime');
    Aktueller_Datensatz = table2timetable(Aktueller_Datensatz);
        
    if(Mittelwerbereich_Sekunden ~= 0)
        Aktueller_Datensatz = Aktueller_Datensatz(timerange(Aktueller_Datensatz.DateTime_Reset_Zero(1) + seconds(From_Point_Sekunden),...
            seconds(From_Point_Sekunden) + Aktueller_Datensatz.DateTime_Reset_Zero(1) + seconds(Mittelwerbereich_Sekunden), 'closed'), :);
    else
        Aktueller_Datensatz = Aktueller_Datensatz(Aktueller_Datensatz.DateTime_Reset_Zero(1) + seconds(From_Point_Sekunden), :);
    end
    
    [G, SpectrumCodes] = findgroups(Aktueller_Datensatz.Spektrumbezeichnung, Aktueller_Datensatz.Probandencode);
    
    Mittelwerte = table(SpectrumCodes,...
        splitapply(@mean, Aktueller_Datensatz.PupilDiameterLeft, G),...
        splitapply(@(x) x(1), Aktueller_Datensatz.Probandencode, G),...
        'VariableNames',{'Spektrumbezeichnung', 'PupilDiameterLeft', 'Probandencode'});    
end