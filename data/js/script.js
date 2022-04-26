function rgbFromCSS(number,color){
    let str = String(number);
    str+="&red=" + parseInt(color.substring(1,3),16);
    str+="&green="+ parseInt(color.substring(3,5),16);
    str+="&blue="+ parseInt(color.substring(5,7),16);
    return str;
}

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
        case "/backlightbrightness":
            value = document.getElementById("backgroundBrightnessSlider").value; break;
        case "/h1color":
        case "/h2color":
        case "/m1color":
        case "/m2color":
        case "/bgcolor":
        case "/bg2color":
        case "/blcolor":
        case "/bl2color":
            value = rgbFromCSS(0,value);
            break;
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
        case "/effectbl":
            value = document.getElementById("backlight").value; 
            modifySiteState();
            break;
        default:
            if(functionCall.substring(0,10)==="/spotlight"){
                value = rgbFromCSS(functionCall.substring(10),value);
                functionCall = "/spotcolor"
                break;
            }
            else{console.log("Case not supported: " + functionCall); return;}
    }

    console.log("GET" + " " + functionCall + "?value=" + value);
    sendData(functionCall,value);
    
}

function sendData(functionCall,value=""){
    let xhttp = new XMLHttpRequest();
    xhttp.onreadystatechange = function() {
        if (this.readyState == 4 && this.status == 200 && functionCall==="/getsettings") {
            loadSettings(this.responseText);
        }else if(this.readyState == 4 && this.status == 200) {
            console.log("Response: " + this.responseText);
            if(functionCall == "/cmd")
                document.getElementById("cmdresponse").innerHTML="Success: " + this.responseText;
        }else if(this.readyState == 4 && this.status == 404){
            console.log("404: No GET request for " +  functionCall +"?value=" + value);
            document.getElementById("cmdresponse").innerHTML="404 on " + functionCall +"?value=" + value;
        }
    };
    if(value!=="") xhttp.open("GET", functionCall + "?value=" + value, true);
    else xhttp.open("GET", functionCall, true);
    xhttp.send();
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
    const colorPickers = document.getElementsByTagName('hex-color-picker');
    for(let i=0;i<colorPickers.length;i++){
        colorPickers[i].color = data[id++];
    }
    //Check if there are any more colour values. If yes, throw a warning
    if(data[id].substring(0,1)==="#") alert("An extra color value was recieved. Did you configure the spotlights?\nContinuing anyways...");
    //Move past colour values and continue
    while(data[id].substring(0,1)==="#") id++;
    document.getElementById("foreground").value = data[id++];
    document.getElementById("background").value = data[id++];
    document.getElementById("spotlightEffect").value = data[id++];
    modifySiteState();
}


function modifySiteState(){
    const autoBrightnessElements = document.getElementsByClassName("hideOnAutoBrightness");
    if(document.getElementById("autobrightness").checked)
        for(let i=0;i<autoBrightnessElements.length;i++){autoBrightnessElements[i].style.display = "none";}
    else for(let i=0;i<autoBrightnessElements.length;i++){autoBrightnessElements[i].style.display = "";}

    const fgPattern = document.getElementById("foreground").value;
    switch(fgPattern){
        case ("off"):
        case ("rainbow"):
            document.getElementById("colorpicker1").style.display = "none";
            document.getElementById("colorpicker2").style.display = "none";
            document.getElementById("colorpicker3").style.display = "none";
            document.getElementById("colorpicker4").style.display = "none";
            break;
        case ("gradient"):
            document.getElementById("text1").innerHTML = "Top Left Color"
            document.getElementById("colorpicker1").style.display = "";
            document.getElementById("text2").innerHTML = "Bottom Right Color"
            document.getElementById("colorpicker2").style.display = "";
            document.getElementById("colorpicker3").style.display = "none";
            document.getElementById("colorpicker4").style.display = "none";
            break;
        //show all
        case ("solid"):
            document.getElementById("text1").innerHTML = "Hours 10"
            document.getElementById("colorpicker1").style.display = "";
            document.getElementById("text2").innerHTML = "Hours 1"
            document.getElementById("colorpicker2").style.display = "";
            document.getElementById("text3").innerHTML = "Minutes 10"
            document.getElementById("colorpicker3").style.display = "";
            document.getElementById("text4").innerHTML = "Minutes 1"
            document.getElementById("colorpicker4").style.display = "";
            break;
        default:
            console.log("Did not find fgPattern for case " + fgPattern);
    }

    const bgPattern = document.getElementById("background").value;
    function showBG(){
        document.getElementById("bgTitle").style.display = "";
        document.getElementById("bg1").style.display = "";
    }
    switch(bgPattern){
        case ("off"):
        case ("rainbow"):
            document.getElementById("bgTitle").style.display = "none";
            document.getElementById("bg1").style.display = "none";
            document.getElementById("bg2").style.display = "none";
            break;
        case ("gradient"):
        case ("fire"):
            showBG();
            document.getElementById("bg2").style.display = "";
            break;
        //show all
        case ("solid"):
        case ("rain"):
        case ("sparkle"):
        case ("loop"):
            showBG();
            document.getElementById("bg2").style.display = "none";
            break;
        default:
            console.log("Did not find bgPattern for case " + bgPattern);
    }

    const slPattern = document.getElementById("spotlightEffect").value;
    const spotlights = document.getElementsByClassName("spotlightContainer");
    function showSL(){
        document.getElementById("slTitle").style.display = "";
        document.getElementById("spotlightSection").style.display = "";
    }
    function iterateSpotlights(start,show=false){for(let i=start;i<spotlights.length;i++) if(show) spotlights[i].style.display = ""; else spotlights[i].style.display = "none";}
    switch(slPattern){
        case ("off"):
        case ("rainbow"):
            document.getElementById("slTitle").style.display = "none";
            document.getElementById("spotlightSection").style.display = "none";
            break;
        //Show 1
        case ("rain"):
        case ("sparkle"):
            showSL();
            spotlights[0].style.display = "";
            iterateSpotlights(1);
            break;
        //Show 2
        case ("gradient"):
        case ("fire"):
            showSL();
            spotlights[0].style.display = "";
            spotlights[1].style.display = "";
            iterateSpotlights(2);
            break;
        //show all
        case ("solid"):
            showSL();
            iterateSpotlights(0,true);
            break;
        default:
            console.log("Did not find slPattern for case " + slPattern);
    }

    const blPattern = document.getElementById("backlight").value;
    function showBL(){
        document.getElementById("blTitle").style.display = "";
        document.getElementById("bl1").style.display = "";
    }
    switch(blPattern){
        case ("off"):
        case ("rainbow"):
            document.getElementById("blTitle").style.display = "none";
            document.getElementById("bl1").style.display = "none";
            document.getElementById("bl2").style.display = "none";
            break;
        case ("gradient"):
        case ("fire"):
            showBL();
            document.getElementById("bl2").style.display = "";
            break;
        //show all
        case ("solid"):
        case ("rain"):
        case ("sparkle"):
        case ("loop"):
            showBL();
            document.getElementById("bl2").style.display = "none";
            break;
        default:
            console.log("Did not find blPattern for case " + bgPattern);
    }
}


function submitcmd(){
    let command = document.getElementById("cmd").value.split(" ");
    if(command.length >= 2){
        console.log("GET /cmd?v=0&c="+command[0] + "&v=" + command[1]);
        sendData("/cmd","0&c="+command[0] + "&v=" + command[1]);
    }else if (command[0].length > 0){
        console.log("GET /cmd?v=0&c="+command[0]);
        sendData("/cmd","0&c="+command[0]);
    }
}
document.getElementById("cmd").addEventListener("keyup", function(event) {if (event.code === "Enter") submitcmd();});
