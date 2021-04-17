let h1 = new KellyColorPicker({place : 'picker1', input : 'color1', size : 150});
let h2 = new KellyColorPicker({place : 'picker2', input : 'color2', size : 150});
let m1 = new KellyColorPicker({place : 'picker3', input : 'color3', size : 150});
let m2 = new KellyColorPicker({place : 'picker4', input : 'color4', size : 150});
let bg = new KellyColorPicker({place : 'picker5', input : 'color5', size : 150});
let bg2 = new KellyColorPicker({place : 'picker6', input : 'color6', size : 150});
let s1 = new KellyColorPicker({place : 'picker10', input : 'color10', size : 100});
let s2 = new KellyColorPicker({place : 'picker11', input : 'color11', size : 100});
let s3 = new KellyColorPicker({place : 'picker12', input : 'color12', size : 100});
let s4 = new KellyColorPicker({place : 'picker13', input : 'color13', size : 100});
let s5 = new KellyColorPicker({place : 'picker14', input : 'color14', size : 100});
let s6 = new KellyColorPicker({place : 'picker15', input : 'color15', size : 100});
let s7 = new KellyColorPicker({place : 'picker16', input : 'color16', size : 100});
let s8 = new KellyColorPicker({place : 'picker17', input : 'color17', size : 100});
let s9 = new KellyColorPicker({place : 'picker18', input : 'color18', size : 100});
let s10 = new KellyColorPicker({place : 'picker19', input : 'color19', size : 100});
let s11 = new KellyColorPicker({place : 'picker20', input : 'color20', size : 100});
let s12 = new KellyColorPicker({place : 'picker21', input : 'color21', size : 100});

document.getElementById("gradientPicker").style.display = 'none';
let lastFunction = "";

function updateClock(functionCall,value=""){
    switch(functionCall){
        case "/power":
            value = document.getElementById("power").checked.toString(); break;
        case "/transparency":
            value = document.getElementById("transparencySlider").value; break;
        case "/autobrightness":
            value = document.getElementById("autobrightness").checked.toString(); 
            modifySiteState();
            break;
        case "/brightness":
            value = document.getElementById("brightnessSlider").value; break;
        case "/spotlightbrightness":
            value = document.getElementById("spotlightBrightnessSlider").value; break;
        case "/backgroundbrightness":
            value = document.getElementById("backgroundBrightnessSlider").value; break;
        case "/h1color":
            value = document.getElementById("color1").style.backgroundColor; break;
        case "/h2color":
            value = document.getElementById("color2").style.backgroundColor; break;
        case "/m1color":
            value = document.getElementById("color3").style.backgroundColor; break;
        case "/m2color":
            value = document.getElementById("color4").style.backgroundColor; break;
        case "/bgcolor":
            value = document.getElementById("color5").style.backgroundColor; break;
        case "/bgcolor2":
            value = document.getElementById("color6").style.backgroundColor; break;
        case "/spot1color":
            value = document.getElementById("color10").style.backgroundColor + ",1"; break;
        case "/spot2color":
            value = document.getElementById("color11").style.backgroundColor + ",2"; break;
        case "/spot3color":
            value = document.getElementById("color12").style.backgroundColor + ",3"; break;
        case "/spot4color":
            value = document.getElementById("color13").style.backgroundColor + ",4"; break;
        case "/spot5color":
            value = document.getElementById("color14").style.backgroundColor + ",5"; break;
        case "/spot6color":
            value = document.getElementById("color15").style.backgroundColor + ",6"; break;
        case "/spot7color":
            value = document.getElementById("color16").style.backgroundColor + ",7"; break;
        case "/spot8color":
            value = document.getElementById("color17").style.backgroundColor + ",8"; break;
        case "/spot9color":
            value = document.getElementById("color18").style.backgroundColor + ",9"; break;
        case "/spot10color":
            value = document.getElementById("color19").style.backgroundColor + ",10"; break;
        case "/spot11color":
            value = document.getElementById("color20").style.backgroundColor + ",11"; break;
        case "/spot12color":
            value = document.getElementById("color21").style.backgroundColor + ",12"; break;
        case "/effectfg":
            value = document.getElementById("foreground").value;
            modifySiteState();
            break;
        case "/effectbg":
            value = document.getElementById("background").value; 
            modifySiteState();
            break;
        case "/effectsl":
            value = document.getElementById("spotlightEffect").value; 
            modifySiteState();
            break;
        default:
            console.log("Case not supported: " + functionCall);
    }
    if(value.length > 4 && value.substring(0,4)==="rgb("){
        let rgb = value.substring(4,value.length).replace(/ /g,"").replace(/\)/,"").split(",");
        if(rgb.length === 3){
            value = "0&red=" + rgb[0] + "&green=" + rgb[1] + "&blue=" + rgb[2];
        }else{
            value = rgb[3] + "&red=" + rgb[0] + "&green=" + rgb[1] + "&blue=" + rgb[2];
            functionCall = "/spotcolor"
        }
    }
    sendData(functionCall,value);
    console.log("GET" + " " + functionCall + "?value=" + value);
}

function sendData(functionCall,value=""){
    let xhttp = new XMLHttpRequest();
    xhttp.onreadystatechange = function() {
        if (this.readyState == 4 && this.status == 200 && this.responseText!=="1") {
            loadSettings(this.responseText);
        }else if(this.readyState == 4 && this.status == 200 && this.responseText==="1") {
            console.log("Request sent successfully");
        }else if(this.readyState == 4 && this.status != 404){
            console.log("Error:\nReady: " + (this.readyState==4) + "\nStatus: " + (this.status) + "\nResponse: " + (this.responseText))
        }else if(this.readyState == 4 && this.status == 404){
            console.log("404: No GET request for " + functionCall)
        }
    };
    if(value!=="") xhttp.open("GET", functionCall + "?value=" + value, true);
    else xhttp.open("GET", functionCall, true);
    xhttp.send();
}

let rangeslider = document.getElementById("brightnessSlider");
let output = document.getElementById("brightness");
rangeslider.oninput = function() {
    output.innerHTML = Math.round(rangeslider.value/255*100) + '%';
}
let spotRangeslider = document.getElementById("spotlightBrightnessSlider");
let spotOutput = document.getElementById("spotlightBrightness");
spotRangeslider.oninput = function() {
    spotOutput.innerHTML = Math.round(spotRangeslider.value/255*100) + '%';
}
let backRangeslider = document.getElementById("backgroundBrightnessSlider");
let backOutput = document.getElementById("backgroundBrightness");
backRangeslider.oninput = function() {
    backOutput.innerHTML = Math.round(backRangeslider.value/255*100) + '%';
}
let transparencyslider = document.getElementById("transparencySlider");
let transparencyOutput = document.getElementById("clockTransparency");
transparencyslider.oninput = function() {
    transparencyOutput.innerHTML = Math.round(transparencyslider.value/255*100) + '%';
}

function loadSettings(settings){
    console.log(settings);
    let id = 0;
    let data = settings.split("|");
    document.getElementById("power").checked = Number(data[id++]);
    document.getElementById("transparencySlider").value = data[id++]; 
    document.getElementById("autobrightness").checked = Number(data[id++]); 
    document.getElementById("brightnessSlider").value = data[id++]; 
    document.getElementById("spotlightBrightnessSlider").value = data[id++];
    document.getElementById("backgroundBrightnessSlider").value = data[id++];
    h1.setColor(data[id++]);
    h2.setColor(data[id++]);
    m1.setColor(data[id++]);
    m2.setColor(data[id++]);
    bg.setColor(data[id++]);
    bg2.setColor(data[id++]);
    s1.setColor(data[id++]);
    s2.setColor(data[id++]);
    s3.setColor(data[id++]);
    s4.setColor(data[id++]);
    s5.setColor(data[id++]);
    s6.setColor(data[id++]);
    s7.setColor(data[id++]);
    s8.setColor(data[id++]);
    s9.setColor(data[id++]);
    s10.setColor(data[id++]);
    s11.setColor(data[id++]);
    s12.setColor(data[id++]);
    document.getElementById("foreground").value = data[id++];
    document.getElementById("background").value = data[id++];
    document.getElementById("spotlightEffect").value = data[id++];
    modifySiteState();
}
function modifySiteState(){
    let ab = document.getElementById("autobrightness");
    let mb = document.getElementById("manualbrightness");
    if(ab.checked === true) mb.style.display = 'none';
    else mb.style.display = 'grid';
    document.getElementById("brightness").innerHTML = Math.round(rangeslider.value/255*100) + '%';
    document.getElementById("backgroundBrightness").innerHTML = Math.round(backgroundBrightnessSlider.value/255*100) + '%';
    document.getElementById("spotlightBrightness").innerHTML = Math.round(spotRangeslider.value/255*100) + '%';
    document.getElementById("clockTransparency").innerHTML = Math.round(transparencyslider.value/255*100) + '%';

    let t1 = document.getElementById("text1"); 
    let t2 = document.getElementById("text2");  
    let cp1 = document.getElementById("colorpicker1"); 
    let cp2 = document.getElementById("colorpicker2"); 
    let cp3 = document.getElementById("colorpicker3"); 
    let cp4 = document.getElementById("colorpicker4"); 
    let fg_ = document.getElementById("foreground");
    if(fg_.value==="gradient"){
        t1.innerHTML = "Top Left Color"
        t2.innerHTML = "Bottom Right Color"
        cp1.style.display='';
        cp2.style.display='';
        cp3.style.display='none';
        cp4.style.display='none';
    }else if (fg_.value==="rainbow" || fg_.value==="off"){
        cp1.style.display='none';
        cp2.style.display='none';
        cp3.style.display='none';
        cp4.style.display='none';
    }else if(fg_.value==="solid"){
        t1.innerHTML = "Hours 10"
        t2.innerHTML = "Hours 1"
        cp1.style.display='';
        cp2.style.display='';
        cp3.style.display='';
        cp4.style.display='';
    }

    
    let bg_ = document.getElementById("background"); 
    let gp = document.getElementById("gradientPicker"); 
    if(bg_.value === "gradient") gp.style.display = '';
    else gp.style.display = 'none';

    let sl_ = document.getElementById("spotlightEffect"); 
    let sl1 = document.getElementById("spotlightColor1"); 
    let sl2 = document.getElementById("spotlightColor2"); 
    let sls = document.getElementsByName("spotlightColors"); 
    
    if(sl_.value === "gradient"){
        sl1.style.display = 'inline';
        sl2.style.display = 'inline';
        sls.forEach(s => {s.style.display = 'none';}) 
    }
    else if(sl_.value === "rain" || sl_.value === "sparkle"){
        sl1.style.display = 'inline';
        sl2.style.display = 'none';
        sls.forEach(s => {s.style.display = 'none';}) 
    }
    else{
        sl1.style.display = 'inline';
        sl2.style.display = 'inline';
        sls.forEach(s => {s.style.display = 'inline';}) 
    }

}

function mouseUp(){
    if(lastFunction!=="") updateClock(lastFunction)
}

function lastColor(command){
    lastFunction = command;
}

sendData("/getsettings");