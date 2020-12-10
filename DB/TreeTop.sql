CREATE DATABASE TreeTop
GO
USE TreeTop
GO
CREATE TABLE [dbo].[Dance]
(
    [DanceId] INT IDENTITY(1,1) NOT NULL CONSTRAINT [PK_Dance_DanceId] PRIMARY KEY,
    [Name] VARCHAR(100) NOT NULL
)
GO
CREATE TABLE [dbo].[ResponseLog]
(
    [ResponseLogId] BIGINT IDENTITY(1,1) NOT NULL CONSTRAINT [PK_ResponseLog_ResponseLogId] PRIMARY KEY,
    [StandByMillis] BIGINT NOT NULL,
    [DanceId] INT NOT NULL CONSTRAINT [FK_ResponseLog_DanceId_Dance_DanceId] FOREIGN KEY REFERENCES [dbo].[Dance](DanceId),
	[CreatedDate] DATETIME NOT NULL CONSTRAINT [DF_ResponseLog_CreatedDate]  DEFAULT (GETDATE())
)
GO
CREATE SCHEMA const
GO
CREATE VIEW [const].[Dance]
AS
	SELECT
		CONVERT(INT, 0) AS [None],
		CONVERT(INT, 1) AS [Starman]
GO

SET IDENTITY_INSERT dbo.Dance ON
	INSERT dbo.Dance (DanceId, Name)
	SELECT d.None, 'None' FROM const.Dance AS d UNION
	SELECT d.Starman, 'Starman' FROM const.Dance AS d
SET IDENTITY_INSERT dbo.Dance OFF

GO
CREATE OR ALTER PROCEDURE [dbo].[ResponseGet]
AS
BEGIN
    DECLARE @Now DATETIME = GETDATE();
    DECLARE @MinHour DATETIME = DATEADD(HOUR, 7, CONVERT(DATETIME, CONVERT(DATE, @Now)));
    DECLARE @MaxHour DATETIME = DATEADD(HOUR, 21, CONVERT(DATETIME, CONVERT(DATE, @Now)));
    DECLARE @MaxStandByMillis BIGINT = 5*1000; --5 Minutes

    DECLARE @StandByMillis BIGINT = 0;
    DECLARE @DanceId INT = 0;
    DECLARE @OddsOfDanceOffOneIn INT = 12;

    IF(@Now < @MinHour )
    BEGIN
        SET @StandByMillis = DATEDIFF_BIG(MILLISECOND, @Now, @MinHour);     
    END
    ELSE IF ( @Now > @MaxHour)
    BEGIN
        SET @StandByMillis = DATEDIFF_BIG(MILLISECOND, @Now, DATEADD(DAY, 1, @MinHour));     
    END
    SET @StandByMillis = IIF(@StandByMillis > @MaxStandByMillis, @MaxStandByMillis, @StandByMillis)

    IF(@StandByMillis = 0)
    BEGIN
        SET @StandByMillis = 5 * 60 * 1000 --Sleep for 5min after playing
        IF( 1 = CONVERT(INT, RAND() * 100) % @OddsOfDanceOffOneIn)
        BEGIN
            SET @DanceId = (SELECT TOP 1 d.DanceId FROM dbo.Dance AS d CROSS JOIN const.Dance AS cd WHERE d.DanceId <> cd.[None] ORDER BY NEWID())
        END
    END

    INSERT dbo.ResponseLog (StandByMillis, DanceId)
    VALUES (@StandByMillis, @DanceId)

    SELECT @StandByMillis AS StandByMillis, @DanceId AS DanceId
END
GO
