#!/bin/bash

./svgtoicns Error.svg
mv Error.icns ../icns/error.icns

./svgtoicns Error_Shared.svg
mv Error_Shared.icns ../icns/error_swm.icns

./svgtoicns OK.svg
mv OK.icns ../icns/ok.icns

./svgtoicns OK_Shared.svg
mv OK_Shared.icns ../icns/ok_swm.icns

./svgtoicns Sync.svg
mv Sync.icns ../icns/sync.icns

./svgtoicns Sync_Shared.svg
mv Sync_Shared.icns ../icns/sync_swm.icns

./svgtoicns Warning.svg
mv Warning.icns ../icns/warning.icns

./svgtoicns Warning_Shared.svg
mv Warning_Shared.icns ../icns/warning_swm.icns
