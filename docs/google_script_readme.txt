// First column in spreadsheet should have header "Timestamp", 
// rest as per requirements e.g. "SensorReading", "Threshold" etc
// Name the sheet (bottom tag), e.g. "Autowater"
// Name and save the sheet e.g. "Data_Logger"
// Use Menu Extensions->Apps Script and create or edit the file "Code.gs" for the "doGet()" function
// and insert this files contents
// Use Deploy -> New Deployment each time you make a change to this script. 

// https://stackoverflow.com/questions/69685813/problem-esp32-send-data-to-google-sheet-through-google-app-script

function doGet(e) {
  try {
    var sheet = SpreadsheetApp.getActive().getSheetByName(e.parameter["id"]);
    var headers = sheet.getRange(1, 1, 1, sheet.getLastColumn()).getValues()[0];
    var d = new Date();
    var values = headers.map(h => h == "Timestamp" ? d.toDateString() + ", " + d.toLocaleTimeString() : e.parameter[h]);
    sheet.getRange(sheet.getLastRow() + 1, 1, 1, values.length).setValues([values]);
  } 
  catch (e) {
    return ContentService.createTextOutput(JSON.stringify(e));
    } 
  finally {
    return ContentService.createTextOutput('success');
    }
}


// same function with lock, if you have multiple remote agents updating the spreadsheet at the same time.
// You will not be able to view the spreadsheet online and see updates happening if you use this version.

function doGet(e) {
  const lock = LockService.getDocumentLock();
  if (lock.tryLock(350000)) {
    try {
      var sheet = SpreadsheetApp.getActive().getSheetByName(e.parameter["id"]);
      var headers = sheet.getRange(1, 1, 1, sheet.getLastColumn()).getValues()[0];
      var d = new Date();
      var values = headers.map(h => h == "Timestamp" ? d.toDateString() + ", " + d.toLocaleTimeString() : e.parameter[h]);
      sheet.getRange(sheet.getLastRow() + 1, 1, 1, values.length).setValues([values]);
    } catch (e) {
      return ContentService.createTextOutput(JSON.stringify(e));
    } finally {
      lock.releaseLock();
      return ContentService.createTextOutput('success');
    }
  } else {
    return ContentService.createTextOutput("timeout");
  }
}


// When you select "New Deployment", you will get a popup showing the new spreadsheet ID for access
// Make a copy for your upload code and for testing spreadsheet update via browser

// for upload code
static String GS_ID = "spreadsheet id goes here";

// url for testing in browser (works in Chrome and Firefox)
https://script.google.com/macros/s/spreadsheet_id_goes_here/exec?id=AutoWater&Date=Nov15&Time=20:49&SensorReading=356&SensorThreshold=360&OnTimeSeconds=0&BatteryVoltage=3.8&SuperCapVoltage=17.9





