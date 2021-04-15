function Long_PupilSmooth_Quasi_V1 = Struct_to_Long(PupilSmooth_Table_Quasi)
    
    VariableNamen = fieldnames(PupilSmooth_Table_Quasi);
    
    for Index = 1:size(VariableNamen, 1)
        Aktueller_Datensatz = PupilSmooth_Table_Quasi.(cell2mat(VariableNamen(Index)));
        
        Probanden_Datei_Name = cell2mat(VariableNamen(Index));
        
        % Neue Zeitspalte Spalte: Wird nach jedem Spektrumcode wieder auf 0 runtergezogen
        % Es wird von jeder Kategorie der erste Wert genommen und von allen anderen Werten abgezogen
        % Es ging nur wenn man das ganze als Cell durchführt, daher nochmal cell2mat damit es zusammenkommt
        Aktueller_Datensatz.DateTime_Reset_Zero = cell2mat(splitapply(@(x) {posixtime(x) - posixtime(x(1))},...
            Aktueller_Datensatz.Datetime, findgroups(Aktueller_Datensatz.SpektrumCode)));
        
        Aktueller_Datensatz.DateTime_Reset_Zero = round(Aktueller_Datensatz.DateTime_Reset_Zero,2);
        
        % Spalte zurückwandeln zu einem datetime object
        Aktueller_Datensatz.DateTime_Reset_Zero = datetime(Aktueller_Datensatz.DateTime_Reset_Zero,...
            'convertfrom', 'posixtime', 'Format', 'dd.MM.yyyy HH:mm:ss.SSS', 'TimeZone', 'Europe/Zurich');
        
        % Umwandeln der kategorien wieder zurück in ein cellstring, da Reihenfolge jetzt anders ist
        Aktueller_Datensatz.SpektrumCode = cellstr(Aktueller_Datensatz.SpektrumCode(:));
        Aktueller_Datensatz.Spektrumbezeichnung = cellstr(Aktueller_Datensatz.Spektrumbezeichnung(:));
        Aktueller_Datensatz.Versuchsbezeichnung = cellstr(Aktueller_Datensatz.Versuchsbezeichnung(:));
        Aktueller_Datensatz.Probandencode = cellstr(Aktueller_Datensatz.Probandencode(:));
        % ----------------------------------
        
        % Wieder zurückwandeln
        Aktueller_Datensatz.SpektrumCode =  categorical(Aktueller_Datensatz.SpektrumCode(:));
        Aktueller_Datensatz.Spektrumbezeichnung = categorical(Aktueller_Datensatz.Spektrumbezeichnung(:));
        Aktueller_Datensatz.Versuchsbezeichnung = categorical(Aktueller_Datensatz.Versuchsbezeichnung(:));
        Aktueller_Datensatz.Probandencode = categorical(Aktueller_Datensatz.Probandencode(:));
        % ----------------------------------
        
        PupilSmooth_Table_Quasi.(cell2mat(VariableNamen(Index))) = Aktueller_Datensatz;
    end
    
    ZwischenVariable = struct2cell(PupilSmooth_Table_Quasi);
    Long_PupilSmooth_Quasi_V1 = vertcat(ZwischenVariable{:});
    
end