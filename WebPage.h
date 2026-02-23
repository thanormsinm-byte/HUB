#ifndef WEBPAGE_H
#define WEBPAGE_H

#include <Arduino.h>
#include <vector>
#include "config.h" 

String getDashboardHTML(std::vector<Device>& devices) {
    String html = "<!DOCTYPE html><html lang='th'><head>";
    html += "<meta charset='UTF-8'><meta name='viewport' content='width=device-width, initial-scale=1.0'>";
    html += "<title>Smart Farm Hub Dashboard</title>";
    
    html += "<style>";
    html += "body { font-family: 'Segoe UI', Tahoma, sans-serif; background: #f0f4f8; margin: 0; padding: 25px; }";
    html += ".container { max-width: 1400px; margin: auto; background: white; padding: 30px; border-radius: 12px; box-shadow: 0 4px 20px rgba(0,0,0,0.05); }";
    html += "h2 { border-left: 4px solid #007bff; padding-left: 15px; margin-bottom: 30px; color: #1e293b; font-size: 24px; }";
    
    html += ".table-control { display: flex; justify-content: space-between; align-items: center; margin-bottom: 25px; font-size: 14px; color: #666; }";
    html += ".search-box { padding: 6px 12px; border: 1px solid #ddd; border-radius: 4px; width: 220px; margin-left: 8px; outline: none; }";
    html += "select { padding: 4px 8px; border: 1px solid #ddd; border-radius: 4px; margin: 0 5px; cursor: pointer; }";

    html += "table { width: 100%; border-collapse: collapse; }";
    html += "th { background: #f8fafc; color: #475569; text-align: left; padding: 15px 12px; border-bottom: 2px solid #edf2f7; font-size: 12px; letter-spacing: 0.5px; text-transform: uppercase; }";
    html += "td { padding: 20px 12px; border-bottom: 1px solid #edf2f7; font-size: 14px; color: #334155; vertical-align: middle; }"; 
    
    html += ".btn-orange { background: #ff9800; color: white; border: none; padding: 8px 22px; border-radius: 5px; cursor: pointer; font-weight: bold; font-size: 12px; }";
    html += ".btn-gray { background: #cbd5e1; color: #64748b; border: none; padding: 8px 22px; border-radius: 5px; cursor: not-allowed; font-weight: bold; font-size: 12px; opacity:0.6; }";
    
    html += ".progress-container { width: 100%; background: #e2e8f0; border-radius: 15px; display:none; margin-top: 5px; position: relative; height: 18px; overflow: hidden; }";
    html += ".progress-bar { width: 0%; height: 100%; background: #22c55e; border-radius: 15px; transition: 0.3s; }";
    html += ".progress-text { position: absolute; width: 100%; text-align: center; font-size: 10px; font-weight: bold; color: #1e293b; top: 50%; left: 50%; transform: translate(-50%, -50%); }";
    
    html += ".btn-del { color: #94a3b8; background: none; border: none; cursor: pointer; font-size: 10px; margin-left: 5px; }";
    html += ".dot { height: 9px; width: 9px; border-radius: 50%; display: inline-block; margin-right: 6px; }";
    html += ".active-dot { background-color: #28a745; }"; 
    html += ".offline-dot { background-color: #ff0000; }"; 
    html += ".text-orange { color: #ff9800; font-weight: bold; }"; 

    html += ".pagination { display: flex; justify-content: center; align-items: center; margin-top: 30px; gap: 20px; color: #666; font-size: 13px; }";
    html += ".page-nav { cursor: pointer; user-select: none; }";
    html += ".page-nav.disabled { color: #ccc; cursor: not-allowed; }";
    html += "</style></head><body>";

    html += "<div class='container'><h2>Smart Farm Monitoring Hub</h2>";
    
    html += "<div class='table-control'>";
    html += "<div>Show <select id='entrySelect' onchange='resetPageAndTable()'><option value='20'>20</option><option value='50'>50</option><option value='100'>100</option></select> entries</div>";
    html += "<div>Search: <input type='text' id='searchInput' class='search-box' placeholder='Device ID or IP' onkeyup='resetPageAndTable()'></div>";
    html += "</div>";

    html += "<table id='deviceTable'><thead><tr>";
    html += "<th>NO.</th><th>DEVICE ID</th><th>IP ADDRESS</th><th>STATUS</th><th>FIRMWARE (CUR/LATEST)</th><th>BUILD DATE (CUR/LATEST)</th><th>LAST SEEN</th><th>FORCE UPDATE</th>";
    html += "</tr></thead><tbody id='tableBody'>";

    int no = 1;
    for (const auto& d : devices) {

        bool isTimedOut = (millis() - d.lastTick > 45000);
        bool isActive = (d.status == "Active" && !isTimedOut);
        bool isLatest = (d.curVer == d.lastVer && d.lastVer != "" && d.lastVer != "-");
        
        String safeId = d.id;
        safeId.replace(" ", "_"); safeId.replace("(", ""); safeId.replace(")", ""); safeId.replace(".", "_");

        html += "<tr>";
        html += "<td>" + String(no++) + "</td>";
        html += "<td><strong>" + d.id + "</strong> <button class='btn-del' onclick=\"deleteDevice('" + d.id + "')\">[DEL]</button></td>";
        html += "<td>" + d.ip + "</td>";
        html += "<td><span class='dot " + String(isActive ? "active-dot" : "offline-dot") + "'></span><span>" + String(isActive ? "Active" : "Offline") + "</span></td>";
        html += "<td>" + d.curVer + " / " + d.lastVer + "</td>";
        html += "<td>" + d.curBuild + " / " + d.lastBuild + "</td>";
        html += "<td>" + String((millis() - d.lastTick)/1000) + "s ago</td>";
        html += "<td>";

        if (isActive) {
            html += "<div id='box-" + safeId + "'><button id='btn-" + safeId + "' class='" + String(isLatest ? "btn-gray" : "btn-orange") + "' onclick=\"runUpdate('" + safeId + "', '" + d.ip + "', '" + d.id + "')\">Update</button></div>";
            html += "<div id='pg-" + safeId + "' class='progress-container'><div id='bar-" + safeId + "' class='progress-bar'></div><div id='txt-" + safeId + "' class='progress-text'>0%</div></div>";
        } else {
            html += "-";
        }

        html += "</td></tr>";
    }

    html += "</tbody></table>";
    html += "<div class='pagination'>";
    html += "<span id='prevBtn' class='page-nav' onclick='changePage(-1)'>Previous</span>";
    html += "<span id='pageInfo' style='font-weight:bold; color:#333'>page 1 of 1</span>";
    html += "<span id='nextBtn' class='page-nav' onclick='changePage(1)'>Next</span>";
    html += "</div></div>";

    html += "<script>";

    html += "let hubUpdating=false;";

    html += "function checkHubState(){";
    html += " fetch('/hub_state').then(r=>r.json()).then(d=>{";
    html += "   hubUpdating=d.updating;";
    html += "   document.querySelectorAll('button[id^=\"btn-\"]').forEach(btn=>{";
    html += "     if(hubUpdating){ btn.className='btn-gray'; btn.disabled=true; }";
    html += "     else{ btn.disabled=false; if(!btn.classList.contains('btn-gray')) btn.className='btn-orange'; }";
    html += "   });";
    html += " });";
    html += "}";

    html += "setInterval(checkHubState,2000);";

    html += "function runUpdate(safeId,ip,realId){";
    html += " if(hubUpdating){ alert('Hub กำลังอัปเดตตัวเอง'); return; }";
    html += " if(!confirm('สั่ง Force Update ไปยัง '+realId+' ?')) return;";
    html += " const btnBox=document.getElementById('box-'+safeId);";
    html += " const pgBox=document.getElementById('pg-'+safeId);";
    html += " const bar=document.getElementById('bar-'+safeId);";
    html += " const txt=document.getElementById('txt-'+safeId);";
    html += " btnBox.style.display='none'; pgBox.style.display='block';";
    html += " fetch('/execute_ota?ip='+ip).then(()=>{";
    html += "  let itv=setInterval(()=>{";
    html += "   fetch('/hub_check_status?id='+encodeURIComponent(realId)).then(r=>r.json()).then(data=>{";
    html += "    let p=parseInt(data.progress);";
    html += "    bar.style.width=p+'%'; txt.innerText=p+'%';";
    html += "    if(p>=100){ clearInterval(itv); setTimeout(()=>location.reload(),2000);} ";
    html += "   });";
    html += "  },1500);";
    html += " });";
    html += "}";

    html += "function deleteDevice(id){ if(confirm('Delete '+id+'?')) fetch('/hub_delete_device?id='+encodeURIComponent(id)).then(()=>location.reload()); }";
    html += "window.onload=function(){ updateTable(); checkHubState(); };";

    html += "</script></body></html>";

    return html;
}
#endif
