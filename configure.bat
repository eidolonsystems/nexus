@ECHO OFF
SETLOCAL
SET ROOT=%cd%
IF NOT EXIST configure.bat (
  ECHO @ECHO OFF > configure.bat
  ECHO CALL "%~dp0configure.bat" %%* >> configure.bat
)
IF NOT EXIST build.bat (
  ECHO @ECHO OFF > build.bat
  ECHO CALL "%~dp0build.bat" %%* >> build.bat
)
CALL:configure Nexus %*
CALL:configure WebApi %*
CALL:configure Applications\AdministrationServer %*
CALL:configure Applications\AsxItchMarketDataFeedClient %*
CALL:configure Applications\ChartingServer %*
CALL:configure Applications\ChiaMarketDataFeedClient %*
CALL:configure Applications\ComplianceServer %*
CALL:configure Applications\CseMarketDataFeedClient %*
CALL:configure Applications\CtaMarketDataFeedClient %*
CALL:configure Applications\DefinitionsServer %*
CALL:configure Applications\MarketDataRelayServer %*
CALL:configure Applications\MarketDataServer %*
CALL:configure Applications\ReplayMarketDataFeedClient %*
CALL:configure Applications\RiskServer %*
CALL:configure Applications\SimulationMarketDataFeedClient %*
CALL:configure Applications\SimulationOrderExecutionServer %*
CALL:configure Applications\Spire %*
CALL:configure Applications\TmxIpMarketDataFeedClient %*
CALL:configure Applications\TmxTl1MarketDataFeedClient %*
CALL:configure Applications\UtpMarketDataFeedClient %*
CALL:configure Applications\WebPortal %*
ENDLOCAL
EXIT /B %ERRORLEVEL%

:configure
IF NOT EXIST "%~1" (
  MD "%~1"
)
PUSHD "%~1"
CALL "%~dp0%~1\configure.bat" -DD="%ROOT%\Dependencies" %~2 %~3 %~4 %~5 %~6 %~7
POPD
EXIT /B 0
