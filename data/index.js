var updateWiFiStatusInProgress = false;

function toHHMMSS(time) {
    var sec_num = parseInt(time, 10);
    var hours   = Math.floor(sec_num / 3600);
    var minutes = Math.floor((sec_num - (hours * 3600)) / 60);
    var seconds = sec_num - (hours * 3600) - (minutes * 60);

    if (hours   < 10) {hours   = "0"+hours;}
    if (minutes < 10) {minutes = "0"+minutes;}
    if (seconds < 10) {seconds = "0"+seconds;}
    return hours+':'+minutes+':'+seconds;
}

function loadSettings() {
    $.ajax({
        type: 'GET',
        url: '/api/settings',
        dataType: 'json',
        success: function (data) {
            $('form#wifi input[name=wifi-ssid]').val(data.wifiSSID);
            $('form#wifi input[name=wifi-password]').val(data.wifiPassword);

            $('form#mqtt input[name=mqttHost]').val(data.mqttHost);
            $('form#mqtt input[name=mqttPort]').val(data.mqttPort);
            $('form#mqtt input[name=mqttLogin]').val(data.mqttLogin);
            $('form#mqtt input[name=mqttPassword]').val(data.mqttPassword);
            $('form#mqtt input[name=mqttCommandTopic]').val(data.mqttCommandTopic);
            $('form#mqtt input[name=mqttLightSwitchCommandTopic]').val(data.mqttLightSwitchCommandTopic);
            $('form#mqtt input[name=mqttStateTopic]').val(data.mqttStateTopic);
            $('form#mqtt input[name=mqttHADiscoveryPrefix]').val(data.mqttHADiscoveryPrefix);

            if (data.mqttIsHADiscovery) {
                $('form#mqtt input[name=mqttIsHADiscovery]').prop('checked', true);
            }
        },
        error: function (xhr, str) {
            alert('Errors while loading settings');
        }
    });
}

function loadModbusSettings() {
    $.ajax({
        type: 'GET',
        url: '/api/settings/modbus',
        dataType: 'json',
        success: function (data) {
            $('form#modbus-settings select[name=modbusSpeed]').val(data.modbusSpeed);
            $('form#modbus-settings input[name=addressWBMSW]').val(data.addressWBMSW);
            $('form#modbus-settings input[name=addressWBLED1]').val(data.addressWBLED1);
            $('form#modbus-settings input[name=addressWBLED2]').val(data.addressWBLED2);
            $('form#modbus-settings input[name=addressWBM1W2]').val(data.addressWBM1W2);
            $('form#modbus-settings input[name=addressMTD262MB]').val(data.addressMTD262MB);
        },
        error: function (xhr, str) {
            alert('Errors while loading settings');
        }
    });
}

function updateWiFiStatus() {
    if (updateWiFiStatusInProgress) {
        return;
    }

    updateWiFiStatusInProgress = true;

    $.ajax({
        type: 'GET',
        url: '/api/status',
        dataType: 'json',
        success: function (data) {
            $('#wifi-status').html(data.wifiStatus);
            $('#uptime').html(toHHMMSS(data.uptime));
            $('#freeHeap').html(data.freeHeap + " bytes");
            $('#lastResetReason').html(data.lastResetReason);
        },
        complete: function() {
            updateWiFiStatusInProgress = false;
        }
    });
}

$(function() {
    loadSettings();
    updateWiFiStatus();
    loadModbusSettings()

    setInterval(updateWiFiStatus, 10000);

    $('a.nav-link').click(function(e) {
        e.preventDefault();
        var href = $(this).attr('href');
        if (href == "#statistic") {
            $('#statistic').show();
            $('#settings').hide();
        } else {
            $('#statistic').hide();
            $('#settings').show();
        }
    })

    $('form#wifi').submit(function(event) {
        event.preventDefault();

        $.ajax({
            type: 'POST',
            url: '/api/settings/wifi',
            dataType: 'json',
            data: {
                wifiSSID: $(this).find('input[name=wifi-ssid]').val(),
                wifiPassword: $(this).find('input[name=wifi-password]').val()
            },
            success: function (data) {
                alert('Settings successful changed. Reboot...');

                $.ajax({
                    type: 'POST',
                    url: '/api/reboot',
                    dataType: 'json'
                });
            },
            error: function (xhr, str) {
                var data = JSON.parse(xhr.responseText);
                alert('Errors while save settings: ' + data.message);
            }
        });

        return false;
    });

    $('form#mqtt').submit(function(event) {
        event.preventDefault();

        $.ajax({
            type: 'POST',
            url: '/api/settings/mqtt',
            dataType: 'json',
            data: {
                host: $(this).find('input[name=mqttHost]').val(),
                port: $(this).find('input[name=mqttPort]').val(),
                login: $(this).find('input[name=mqttLogin]').val(),
                password: $(this).find('input[name=mqttPassword]').val(),
                haDiscoveryPrefix: $(this).find('input[name=mqttHADiscoveryPrefix]').val(),
                mqttIsHADiscovery: $(this).find('input[name=mqttIsHADiscovery]').is(':checked'),
                commandTopic: $(this).find('input[name=mqttCommandTopic]').val(),
                lightSwitchCommandTopic: $(this).find('input[name=mqttLightSwitchCommandTopic]').val(),
                stateTopic: $(this).find('input[name=mqttStateTopic]').val()
            },
            success: function (data) {
                alert('Settings successful changed. Reboot...');

                $.ajax({
                    type: 'POST',
                    url: '/api/reboot',
                    dataType: 'json'
                });
            },
            error: function (xhr, str) {
                var data = JSON.parse(xhr.responseText);
                alert('Errors while save settings: ' + data.message);
            }
        });

        return false;
    });

    $('form#modbus-settings').submit(function(event) {
        event.preventDefault();

        $.ajax({
            type: 'POST',
            url: '/api/settings/modbus',
            dataType: 'json',
            data: {
                modbusSpeed: $(this).find('select[name=modbusSpeed]').val(),
                addressWBMSW: $(this).find('input[name=addressWBMSW]').val(),
                addressWBLED1: $(this).find('input[name=addressWBLED1]').val(),
                addressWBLED2: $(this).find('input[name=addressWBLED2]').val(),
                addressWBM1W2: $(this).find('input[name=addressWBM1W2]').val(),
                addressMTD262MB: $(this).find('input[name=addressMTD262MB]').val(),
            },
            success: function (data) {
                alert('Modbus settings successful changed. Reboot...');

                $.ajax({
                    type: 'POST',
                    url: '/api/reboot',
                    dataType: 'json'
                });
            },
            error: function (xhr, str) {
                var data = JSON.parse(xhr.responseText);
                alert('Errors while save modbus settings: ' + data.message);
            }
        });

        return false;
    });
});
