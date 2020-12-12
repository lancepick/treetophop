CREATE DATABASE TreeTop
GO
USE TreeTop
GO
CREATE TABLE dbo.Dance (
    DanceId INT          IDENTITY(1, 1) NOT NULL CONSTRAINT PK_Dance_DanceId PRIMARY KEY,
    Name    VARCHAR(100) NOT NULL,
    Track   INT          NOT NULL CONSTRAINT DF_Dance_Track DEFAULT (0)
)
GO
CREATE TABLE dbo.ResponseLog (
    ResponseLogId BIGINT   IDENTITY(1, 1) NOT NULL CONSTRAINT PK_ResponseLog_ResponseLogId PRIMARY KEY,
    StandByMillis BIGINT   NOT NULL,
    DanceId       INT      NOT NULL CONSTRAINT FK_ResponseLog_DanceId_Dance_DanceId FOREIGN KEY REFERENCES dbo.Dance (DanceId),
    CreatedDate   DATETIME NOT NULL CONSTRAINT DF_ResponseLog_CreatedDate DEFAULT (GETDATE())
)
GO
CREATE TABLE dbo.Pixel (
    PixelId INT IDENTITY(1, 1) NOT NULL CONSTRAINT PK_Pixel_PixelId PRIMARY KEY
)
GO
SET IDENTITY_INSERT dbo.Pixel ON
INSERT dbo.Pixel (PixelId)
       SELECT
           ss.value
       FROM
           STRING_SPLIT('1,2,3,4,5,6,7,8,9,10,11,12,13,14,15,16,17,18,19,20,21,22,23,24,25,26,27,28,29,30,31,32,33,34,35,36,37,38,39,40,41,42,43,44,45,46,47,48,49,50,51,52,53,54,55,56,57,58,59,60,61,62,63,64,65,66', ',') AS ss
           LEFT JOIN dbo.Pixel AS p
               ON p.PixelId = ss.value
       WHERE
           p.PixelId IS NULL
SET IDENTITY_INSERT dbo.Pixel OFF
GO
CREATE TABLE dbo.DanceStep (
    DanceStepId INT IDENTITY(1, 1) NOT NULL CONSTRAINT PK_DanceStep_DanceStepId PRIMARY KEY,
    DanceId     INT NOT NULL CONSTRAINT FK_DanceStep_DanceId_Dance_DanceId FOREIGN KEY REFERENCES dbo.Dance (DanceId),
    StepOrder   INT NOT NULL,
    Pixel       INT NOT NULL,
    R           INT NOT NULL,
    G           INT NOT NULL,
    B           INT NOT NULL,
    Wait        INT NOT NULL,
    Show        BIT NOT NULL
)
GO
CREATE SCHEMA const
GO
CREATE VIEW const.Dance
AS
    SELECT
        CONVERT(INT, 0) AS None,
        CONVERT(INT, 1) AS Starman,
        CONVERT(INT, 2) AS Kart
GO
SET IDENTITY_INSERT dbo.Dance ON
    --SQL Prompt Formatting Off
    INSERT dbo.Dance (DanceId, Name, Track)
    SELECT d.None, 'None', -1 FROM const.Dance AS d UNION
    SELECT d.Starman, 'Starman', 1 FROM const.Dance AS d UNION
    SELECT d.Kart, 'Kart', 2 FROM const.Dance AS d
    --SQL Prompt Formatting On`
SET IDENTITY_INSERT dbo.Dance OFF
GO
IF NOT EXISTS (SELECT 1 FROM dbo.DanceStep)
BEGIN
    --SQL Prompt Formatting Off
    INSERT dbo.DanceStep (DanceId, StepOrder, Pixel, R, G, B, Wait, Show)
    SELECT d.Starman,  1,  -1, 136,112,  0,   50, 1 FROM const.Dance AS d UNION
    SELECT d.Starman,  2,  -1, 216, 40,  0,   50, 1 FROM const.Dance AS d UNION
    SELECT d.Starman,  3,  -1, 200, 76, 12,   50, 1 FROM const.Dance AS d UNION
    SELECT d.Starman,  4,  -1,   0,  0,  0,   50, 1 FROM const.Dance AS d UNION
    SELECT d.Starman,  5,  -1, 216, 40,  0,   50, 1 FROM const.Dance AS d UNION
    SELECT d.Starman,  6,  -1, 252,216,168,   50, 1 FROM const.Dance AS d UNION
    SELECT d.Starman,  7,  -1, 252,152, 56,   50, 1 FROM const.Dance AS d UNION
    SELECT d.Starman,  8,  -1,   0,168,  0,   50, 1 FROM const.Dance AS d UNION
    SELECT d.Starman,  9,  -1, 252,152, 56,   50, 1 FROM const.Dance AS d UNION
    SELECT d.Starman, 10,  -1, 216, 40,  0,   50, 1 FROM const.Dance AS d UNION
    SELECT d.Kart,     1,  -1, 255,  0,  0, 1200, 1 FROM const.Dance AS d UNION
    SELECT d.Kart,     2,  -1, 237,109,  0, 1200, 1 FROM const.Dance AS d UNION
    SELECT d.Kart,     3,  -1,   0,255,  0, 1200, 1 FROM const.Dance AS d 
    --SQL Prompt Formatting On
END
GO
CREATE OR ALTER PROCEDURE dbo.ResponseGet
AS
BEGIN
    DECLARE @Now DATETIME = GETDATE();
    DECLARE @MinHour DATETIME = DATEADD(HOUR, 7, CONVERT(DATETIME, CONVERT(DATE, @Now)));
    DECLARE @MaxHour DATETIME = DATEADD(HOUR, 21, CONVERT(DATETIME, CONVERT(DATE, @Now)));
    DECLARE @MaxStandByMillis BIGINT = 5 * 1000; --5 Minutes

    DECLARE @StandByMillis BIGINT = 0;
    DECLARE @DanceId INT = 0;
    DECLARE @OddsOfDanceOffOneIn INT = 12;

    IF (@Now < @MinHour)
    BEGIN
        SET @StandByMillis = DATEDIFF_BIG(MILLISECOND, @Now, @MinHour);
    END
    ELSE IF (@Now > @MaxHour)
    BEGIN
        SET @StandByMillis = DATEDIFF_BIG(MILLISECOND, @Now, DATEADD(DAY, 1, @MinHour));
    END
    SET @StandByMillis = IIF(@StandByMillis > @MaxStandByMillis, @MaxStandByMillis, @StandByMillis)

    IF (@StandByMillis = 0)
    BEGIN
        SET @StandByMillis = 5 * 60 * 1000 --Sleep for 5min after playing
        IF (1 = CONVERT(INT, RAND() * 100) % @OddsOfDanceOffOneIn)
        BEGIN
            SET @DanceId = (
                SELECT TOP 1 d.DanceId FROM dbo.Dance AS d CROSS JOIN const.Dance AS cd WHERE d.DanceId <> cd.None ORDER BY NEWID()
            )
        END
    END

    INSERT dbo.ResponseLog (StandByMillis, DanceId)
    VALUES (@StandByMillis, @DanceId)

    --SQL Prompt Formatting Off
    SELECT
        @StandByMillis AS StandByMillis,
        d.Track AS Track,
        STRING_AGG(CONVERT(VARCHAR(MAX), CONCAT(ds.Pixel, FORMAT(ds.R, '000'), FORMAT(ds.G, '000'), FORMAT(ds.B, '000'), FORMAT(ds.Wait, '00000'), ds.Show)), ',')WITHIN GROUP(ORDER BY ds.StepOrder) AS Steps
    FROM
        dbo.Dance AS d
        LEFT JOIN dbo.DanceStep AS ds
            ON ds.DanceId = d.DanceId
    WHERE
        d.DanceId = @DanceId
	GROUP BY
		d.Track
    --SQL Prompt Formatting On
END
GO