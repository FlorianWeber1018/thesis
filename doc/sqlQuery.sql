SELECT
  Mitarbeiter.Vorname, Mitarbeiter.Nachname,
  Mitarbeiter.Telefonnummer, Mitarbeiter.EmailAdresse
    FROM Abteilung
  LEFT JOIN Mitarbeiter 
    ON Mitarbeiter.AbteilungID = Abteilung.AbteilungID
WHERE Abteilung.Bezeichnung = 'HW-Entwicklung';