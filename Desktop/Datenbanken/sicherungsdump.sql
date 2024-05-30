PRAGMA foreign_keys=OFF;
BEGIN TRANSACTION;
CREATE TABLE freunde(name varchar(20), vorname varchar(20), strasse varchar(20), plz varchar(20), stadt varchar(20));
INSERT INTO freunde VALUES('Meier','Simon','Am Bleichhacker','79183','Waldkirch');
INSERT INTO freunde VALUES('John','Julian','Dettenbachstrasse','79183','Waldkirch');
INSERT INTO freunde VALUES('Mueller','Carmen','Herrengraben','79263','Simonswald');
INSERT INTO freunde VALUES('Fehrenbach','Lisa','Nonnenbach','79263','Simonswald');
INSERT INTO freunde VALUES('Schneider','Tobias','Riedweg','79297','Winden im Elztal');
COMMIT;
