//##################################################################################################
//      GLOBALE VARIABLEN
//##################################################################################################

const maxNumberMowPoints    = 591;
const useTelegram           = true; // bool set true to use Telegram for control
const useECharts            = true; // bool set true to use plot diagram for battery current and voltage
const name_Ardumower        = 'Sven';

const sector_Name       = ['Alles'  ,'Terrasse' ,'Teich','Schaukel' ,'Einfahrt' ,'Kanninchen'];
const sector_Start      = [0        ,0          ,0.25    ,0.41        ,0.75        ,0.5         ];
const sector_Stop       = [0.85        ,0.26        ,0.42    ,0.76        ,0.85        ,0.6         ];

//##################################################################################################
//      Calculate relative Mowpoint
//##################################################################################################

on({id: 'mqtt.0.Ardumower.iob.stats.mow_point_index'/*Ardumower/iob/stats/mow point index*/, change:"ne" }, function (obj) 
{
    setState('javascript.0.Ardumower.mow_point_relative',Number(obj.state.val) / maxNumberMowPoints);
});

//##################################################################################################
//      Check Max Sector border
//##################################################################################################

on({id: 'javascript.0.Ardumower.mow_point_relative'/*relative Mowpoint*/, change:"ne" }, function (obj) 
{
    const actual_Sector         = getState('javascript.0.Ardumower.mowing_sector'/*mowing sector*/).val;
    const actual_Sector_Index   = sector_Name.indexOf(actual_Sector);

    if (actual_Sector_Index != -1 && getState('mqtt.0.Ardumower.iob.stats.job'/*Ardumower/iob/stats/job*/).val == "mow")
    {
        if (useECharts == true) set_EChart_Points();
        if (obj.state.val > sector_Stop[actual_Sector_Index]) setState('mqtt.0.Ardumower.iob.command.set_mower_stop'/*Ardumower/iob/command/set mower stop*/, "true");
    }
});

//##################################################################################################
//      StartArdumower
//##################################################################################################

function startArdumower(reset_Mowpoint)
{
    if (getState('mqtt.0.Ardumower.iob.stats.position.solution'/*Ardumower/iob/stats/position/solution*/).val == "fix")
    {
        if (reset_Mowpoint == false) setMowSector(99);
        setStateAsync('mqtt.0.Ardumower.iob.command.set_mowMotor_enable'/*Ardumower/iob/command/set mowMotor enable*/, 'true');
        setStateAsync('mqtt.0.Ardumower.iob.command.set_mower_start'/*Ardumower/iob/command/set mower start*/, 'true');
        setStateAsync('mqtt.0.Ardumower.iob.command.set_sonar_enable'/*Ardumower/iob/command/set sonar enable*/, 'true');
        
        if (useECharts == true && reset_Mowpoint == true) reset_ECharts();
        if (useTelegram == false) return;

        sendTo("telegram.0", "send", 
        {
            text:  name_Ardumower + ' fängt dann mal an.',
        });
    }
    else
    {
        if (useTelegram == false) return;

        sendTo("telegram.0", "send", 
        {
            text:  name_Ardumower + ' hat keinen Fix.',
        });
        
    }
}

//##################################################################################################
//      MähSektor auswählen
//##################################################################################################

function setMowSector(sectorItem)
{
    if (sectorItem == 99)
    {
        const actual_Sector = getState('javascript.0.Ardumower.mowing_sector'/*mowing sector*/).val;
        sectorItem          = sector_Name.indexOf(actual_Sector);
    }
    else
    {
        setStateAsync('mqtt.0.Ardumower.iob.command.set_mowPoint'/*Ardumower/iob/command/set mowPoint*/, sector_Start[sector_Name.indexOf(sectorItem)])
    }
    
    setStateAsync('javascript.0.Ardumower.mowing_sector'/*mowing sector*/, sector_Name[sectorItem]);
    console.log("setze MowSector " + sectorItem);
}

//##################################################################################################
//      StopArdumower
//##################################################################################################

function stopArdumower()
{
    setStateAsync('mqtt.0.Ardumower.iob.command.set_mower_stop'/*Ardumower/iob/command/set mower stop*/, 'true');

    if (useTelegram == false) return;

    sendTo("telegram.0", "send", 
    {
        text:  name_Ardumower + ' wurde gestoppt.',
    });

    if (useECharts == true)
    {
        // Chart vorbereiten und an Telegram-Empfänger schicken
        sendTo('echarts.0', {
            preset: 'echarts.0.Ardumower',
            renderer: 'png',
            width: 1800,                    // default 1024
            height: 900,                    // default 300
            background: '#F8F9F9',          // Background color
            theme: 'light',                 // Theme type: 'light', 'dark'
                fileOnDisk: '/tmp/chart.png'
        }, result => {
            if (result.error) {
                console.error(result.error);
            } else {
                console.log(result.data);
                sendTo('telegram.0','/tmp/chart.png');
            }
        });

        // Chart vorbereiten und an Telegram-Empfänger schicken
        sendTo('echarts.0', {
            preset: 'echarts.0.Ardumower GPS',
            renderer: 'png',
            width: 1800,                    // default 1024
            height: 900,                    // default 300
            background: '#F8F9F9',          // Background color
            theme: 'light',                 // Theme type: 'light', 'dark'
                fileOnDisk: '/tmp/chart1.png'
        }, result => {
            if (result.error) {
                console.error(result.error);
            } else {
                console.log(result.data);
                sendTo('telegram.0','/tmp/chart1.png');
            }
        });
    }
}

//##################################################################################################
//      DockArdumower
//##################################################################################################

function dockArdumower()
{
    setStateAsync('mqtt.0.Ardumower.iob.command.set_mower_dock'/*Ardumower/iob/command/set mower dock*/, 'true');

    if (useTelegram == false) return;

    sendTo("telegram.0", "send", 
    {
        text:  name_Ardumower + ' fährt in die Ladestation.',
    });
}

//##################################################################################################
//##################################################################################################
//      TELEGRAMM FUNKTIONEN
//##################################################################################################
//##################################################################################################

//##################################################################################################
//      Hilfsfunktionen
//##################################################################################################

function subsequenceFromStartLast(sequence, at1) 
{
    var start = at1;
    var end = sequence.length - 1 + 1;
    return sequence.slice(start, end);
}

//##################################################################################################
//      Sende Ardumowermenü zu Telegram
//##################################################################################################


function menue_Ardumower(empfaenger)
{
    const actual_status = getState('mqtt.0.Ardumower.iob.stats.job'/*Ardumower/iob/stats/job*/).val
    
    if (actual_status == "idle")
    {
        sendTo('telegram.0', {
            user: (empfaenger),
            text:   'Was kann ich für dich tun? Status ' + name_Ardumower + ': ' + actual_status,
            reply_markup: {
                keyboard: [
                    ['Start', 'Resume', 'Dock'],
                    ['Mow Sector'],
                    ['Parameter'],
                    ['Status'],
                ],
                resize_keyboard:   true,
                one_time_keyboard: true
            }
        });
    }
    else if (actual_status == "mow")
    {
        sendTo('telegram.0', {
            user: (empfaenger),
            text:   'Was kann ich für dich tun? Status ' + name_Ardumower + ': ' + actual_status,
            reply_markup: {
                keyboard: [
                    ['Stop', 'Dock'],
                    ['Mow Sector'],
                    ['Parameter'],
                    ['Status'],
                ],
                resize_keyboard:   true,
                one_time_keyboard: true
            }
        });
    }
    else if (actual_status == "charge")
    {
        sendTo('telegram.0', {
            user: (empfaenger),
            text:   'Was kann ich für dich tun? Status ' + name_Ardumower + ': ' + actual_status,
            reply_markup: {
                keyboard: [
                    ['Start'],
                    ['Mow Sector'],
                    ['Parameter'],
                    ['Status'],
                ],
                resize_keyboard:   true,
                one_time_keyboard: true
            }
        });
    }
    else
    {
        sendTo('telegram.0', {
            user: (empfaenger),
            text:   'Was kann ich für dich tun? Status ' + name_Ardumower + ': ' + actual_status,
            reply_markup: {
                keyboard: [
                    ['Start', 'Resume','Stop', 'Dock'],
                    ['Mow Sector'],
                    ['Parameter'],
                    ['Status'],
                ],
                resize_keyboard:   true,
                one_time_keyboard: true
            }
        });
    }

    console.log("zeige Telegram menue_Raum");
}

//##################################################################################################
//      Sende Ardumower Status zu Telegram
//##################################################################################################

function sendArdumowerStatus(empfaenger)
{   
    console.log("sende Status");

    sendTo("telegram.0", "send", 
    {
        user: (empfaenger),
        text: '<b>Ardumower:</b> \n'
            + '<i>Battery Voltage:</i>      ' + getState('mqtt.0.Ardumower.iob.stats.battery_voltage'/*Ardumower/iob/stats/battery voltage*/).val + ' V\n'
            + '<i>Battery Level:</i>          ' + getState('mqtt.0.Ardumower.iob.stats.battery_level'/*Ardumower/iob/stats/battery level*/).val + ' %\n'
            + '######################\n'
            + '<i>Mower State:</i>           ' + getState('mqtt.0.Ardumower.iob.stats.job'/*Ardumower/iob/stats/job*/).val + ' \n'
            + '<i>Mow Point relative:</i> ' + Number(getState('javascript.0.Ardumower.mow_point_relative'/*relative Mowpoint*/).val*100).toFixed(2) + ' %\n'
            + '<i>Mow Sector:</i>            ' + getState('javascript.0.Ardumower.mowing_sector'/*mowing sector*/).val + ' \n'
            + '######################\n'
            + '<i>GPS Solution:</i>          ' + getState('mqtt.0.Ardumower.iob.stats.position.solution'/*Ardumower/iob/stats/position/solution*/).val + ' \n'
            + '<i>GPS Accuracy:</i>         ' + getState('mqtt.0.Ardumower.iob.stats.position.accuracy'/*Ardumower/iob/stats/position/accuracy*/).val + ' m\n'
            + '<i>GPS Age:</i>                  ' + getState('mqtt.0.Ardumower.iob.stats.position.age'/*Ardumower/iob/stats/position/age*/).val + ' s\n'
            + '<i>GPS Satelites:</i>          ' + getState('mqtt.0.Ardumower.iob.stats.position.visible_satelites'/*Ardumower/iob/stats/position/visible satelites*/).val + ' \n'
            + '<i>GPS Satelites dGPS:</i> ' + getState('mqtt.0.Ardumower.iob.stats.position.visible_satelites_dgps'/*Ardumower/iob/stats/position/visible satelites dgps*/).val + ' \n'
            + '######################\n',
        parse_mode: "HTML"
    });

    if (useECharts == true)
    {
        // Chart vorbereiten und an Telegram-Empfänger schicken
        sendTo('echarts.0', {
            preset: 'echarts.0.Ardumower',
            renderer: 'png',
            width: 1800,                    // default 1024
            height: 900,                    // default 300
            background: '#F8F9F9',          // Background color
            theme: 'light',                 // Theme type: 'light', 'dark'
                fileOnDisk: '/tmp/chart.png'
        }, result => {
            if (result.error) {
                console.error(result.error);
            } else {
                console.log(result.data);
                sendTo('telegram.0','/tmp/chart.png');
            }
        });

        // Chart vorbereiten und an Telegram-Empfänger schicken
        sendTo('echarts.0', {
            preset: 'echarts.0.Ardumower GPS',
            renderer: 'png',
            width: 1800,                    // default 1024
            height: 900,                    // default 300
            background: '#F8F9F9',          // Background color
            theme: 'light',                 // Theme type: 'light', 'dark'
                fileOnDisk: '/tmp/chart1.png'
        }, result => {
            if (result.error) {
                console.error(result.error);
            } else {
                console.log(result.data);
                sendTo('telegram.0','/tmp/chart1.png');
            }
        });
    }
}

//##################################################################################################
//      Menue MähSektor auswählen
//##################################################################################################

function menue_MowSector(empfaenger)
{
    var OuterArray = []; 
    for( var i in sector_Name ){
        var InnerArray = [];
        var item = {};
        item.text          = "Sektor: " + sector_Name[i];
        InnerArray.push(item);
        OuterArray.push(InnerArray);
    }
    
    
    sendTo('telegram.0', {
        user: (empfaenger),
        text:   'wähle einen Sector',
        reply_markup: {
            keyboard: OuterArray,
            resize_keyboard:   false,
            one_time_keyboard: true
        }
    });

    console.log("zeige Telegram menue_MowSector");
}

//##################################################################################################
//      Menue Parameter auswählen
//##################################################################################################

function menue_Parameter(empfaenger)
{
    
    sendTo('telegram.0', {
        user: (empfaenger),
        text:   'setze einen Parameter',
        reply_markup: {
            keyboard: [
                    ['mowMotor true', 'mowMotor false'],
                    ['sonar true', 'sonar false'],
                    ['finish and restart true', 'finish and restart false'],
                    ['skip Waypoiint']
                ],
            resize_keyboard:   false,
            one_time_keyboard: true
        }
    });

    console.log("zeige Telegram menue_Parameter");
}

//##################################################################################################
//      Send Error Value to Telegram
//##################################################################################################

on({id: 'mqtt.0.Ardumower.iob.error'/*Ardumower/iob/error*/, change:"ne" }, function (obj) 
{
    if (useTelegram == false) return;

    sendTo("telegram.0", "send", 
    {
        text:  obj.state.val,
    });
});

//##################################################################################################
//      Telegram Hauptfunktion
//##################################################################################################

on({id: 'telegram.0.communicate.request', change: "any"}, function (obj) 
{
    if (useTelegram == false) return;

    var value = obj.state.val;
    var oldValue = obj.oldState.val;
    var letzter_Absender, vorletzter_Text, letzter_Text, letzter_Text_klein;

    letzter_Absender = (obj.state ? obj.state.val : "").slice(1, ((obj.state ? obj.state.val : "").indexOf(']') + 1) - 1);
    vorletzter_Text = subsequenceFromStartLast((obj.oldState ? obj.oldState.val : ""), (((obj.oldState ? obj.oldState.val : "").indexOf(']') + 1 + 1) - 1));
    letzter_Text = subsequenceFromStartLast((obj.state ? obj.state.val : ""), (((obj.state ? obj.state.val : "").indexOf(']') + 1 + 1) - 1));
    letzter_Text_klein = subsequenceFromStartLast((obj.state ? obj.state.val : ""), (((obj.state ? obj.state.val : "").indexOf(']') + 1 + 1) - 1)).toLowerCase();

    console.log("letzter_Absender: " + letzter_Absender);
    console.log("vorletzter_Text: " + vorletzter_Text);
    console.log("letzter_Text: " + letzter_Text);
    console.log("letzter_Text_klein: " + letzter_Text_klein);

    if (letzter_Text == 'Ardumower' || letzter_Text_klein == 'ardumower') 
    {
        menue_Ardumower(letzter_Absender);
    }

    if (letzter_Text =='Status' == true && vorletzter_Text == 'Ardumower')
    {
        sendArdumowerStatus(letzter_Absender);
    }

    if (letzter_Text =='Start' == true && vorletzter_Text == 'Ardumower')
    {
        startArdumower(true);
    }
    
    if (letzter_Text =='Resume' == true && vorletzter_Text == 'Ardumower')
    {
        startArdumower(false);
    }

    if (letzter_Text =='Stop' == true && vorletzter_Text == 'Ardumower')
    {
        stopArdumower();
    }

    if (letzter_Text =='Dock' == true && vorletzter_Text == 'Ardumower')
    {
        dockArdumower();
    }

    if (letzter_Text =='Parameter' == true && vorletzter_Text == 'Ardumower')
    {
        menue_Parameter();
    }

    if (letzter_Text =='Mow Sector' == true && vorletzter_Text == 'Ardumower')
    {
        menue_MowSector(letzter_Absender);
    }

    if (letzter_Text.includes('Sektor: ') == true && vorletzter_Text == 'Mow Sector')
    {
        setMowSector(letzter_Text.substring(8));
    }
});

//##################################################################################################
//##################################################################################################
//      ECHarts FUNKTIONEN
//##################################################################################################
//##################################################################################################

//##################################################################################################
//      Reset EChart Datapoints
//##################################################################################################

function reset_ECharts()
{
    sendTo('sql.0', 'deleteAll', [
                {id: 'javascript.0.Ardumower.charts_battery_level'},
                {id: 'javascript.0.Ardumower.charts_battery_current'},
                {id: 'javascript.0.Ardumower.charts_gps_accuracy'},
                {id: 'javascript.0.Ardumower.charts_gps_age'}
                ],);// result => console.log('deleted'));
}

//##################################################################################################
//      Set EChart Datapoints
//##################################################################################################

function set_EChart_Points()
{
    console.log("set_EChart_Points");
    
    const mowPoint          = Number(getState("javascript.0.Ardumower.mow_point_relative").val) * 10;
    const mowPoint_Int      = Math.floor(mowPoint);
    const mowPoint_Minute   = Math.round((mowPoint - mowPoint_Int)* 60);
    
    const heute =  new Date(2022,
                            0,
                            1,
                            mowPoint,
                            mowPoint_Minute,
                            0,
                            0);

    sendTo('sql.0', 'storeState', [
            {id: 'javascript.0.Ardumower.charts_battery_level', state: {val: getState('mqtt.0.Ardumower.iob.stats.battery_level'/*Ardumower/iob/stats/battery level*/).val, ts: heute.getTime()}},
            {id: 'javascript.0.Ardumower.charts_battery_current', state: {val: getState('mqtt.0.Ardumower.iob.stats.amps'/*Ardumower/iob/stats/amps*/).val, ts: heute.getTime()}},
            {id: 'javascript.0.Ardumower.charts_gps_accuracy', state: {val: getState('mqtt.0.Ardumower.iob.stats.position.accuracy'/*Ardumower/iob/stats/position/accuracy*/).val, ts: heute.getTime()}},
            {id: 'javascript.0.Ardumower.charts_gps_age', state: {val: getState('mqtt.0.Ardumower.iob.stats.position.age'/*Ardumower/iob/stats/position/age*/).val, ts: heute.getTime()}}
            ],);// result => console.log('storeState'));
}
//##################################################################################################
//      Create Datapoints
//##################################################################################################

function erster_start() {
    log("Ardumower: Erster Start des Skriptes! Datenpunkte werden erstellt!");

    createStateAsync("javascript.0.Ardumower.mow_point_relative", 0, false, {
        name: "relative Mowpoint",
        desc: "relative Mowpoint",
        type: "number",
        role: "value",
        unit: ""
    });

    createStateAsync("javascript.0.Ardumower.mowing_sector", 0, false, {
        name: "mowing_sector",
        desc: "actual_sector to mow",
        type: "string",
        role: "value",
        unit: ""
    });

    if (useECharts == true)
    {
        createStateAsync("javascript.0.Ardumower.charts_battery_level", 0, false, {
            name: "battery_level",
            desc: "battery_level",
            type: "number",
            role: "value",
            unit: ""
        });

        createStateAsync("javascript.0.Ardumower.charts_battery_current", 0, false, {
            name: "battery_current",
            desc: "battery_current",
            type: "number",
            role: "value",
            unit: ""
        });

        createStateAsync("javascript.0.Ardumower.charts_gps_accuracy", 0, false, {
            name: "gps_accuracy",
            desc: "gps_accuracy",
            type: "number",
            role: "value",
            unit: ""
        });

        createStateAsync("javascript.0.Ardumower.charts_gps_age", 0, false, {
            name: "gps_age",
            desc: "gps_age",
            type: "number",
            role: "value",
            unit: ""
        });
    }
}

erster_start();