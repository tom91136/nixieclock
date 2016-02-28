ntprun=loadfile("ntp.lua")

function syncTime()
    ntprun():sync(function(T) 
                        print("@s " .. (T.ustamp) .. "000\n") 
                  end)
end

wifi.setmode(wifi.STATION)
wifi.setphymode(wifi.PHYMODE_N)
wifi.sta.config("<SSID>","<PASSWORD>")
wifi.sta.eventMonReg(wifi.STA_GOTIP, function()
    wifi.sta.eventMonStop("unreg all")
    -- 1 hr
    syncTime()
    tmr.alarm(0, 3600000, tmr.ALARM_AUTO, syncTime) 
   
    
    
end)
wifi.sta.connect()
wifi.sta.eventMonStart()

--apConfig = {
--    ssid="CLOCK", 
--    pwd="1234567", 
--    auth=wifi.AUTH_WPA2_PSK}
--ipConfig = {
--    ip="192.168.1.1",
--    netmask="255.255.255.0",
--    gateway="192.168.1.1"}
--dhcpConfig ={
--    start="192.168.1.100"}
--wifi.ap.config(apConfig)
--wifi.ap.setip(ipConfig)
--wifi.ap.dhcp.config(dhcpConfig)
--wifi.ap.dhcp.start()
