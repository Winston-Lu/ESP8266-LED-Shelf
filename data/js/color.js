//Settings for 24hr option
if(enable24HR && HEIGHT===0 && WIDTH===0){
    HEIGHT = 2;
    WIDTH  = 7;
}else if (HEIGHT===0 && WIDTH===0){
    HEIGHT = 2;
    WIDTH  = 6;
}
const container = document.getElementById('spotlightSection');

for(let i=0;i<HEIGHT;i++){
    let newRow = document.createElement('div');
    newRow.className = "spotlightRow";
    for(let j=0;j<WIDTH;j++){
        const newDiv = document.createElement('div');
        newDiv.className = "spotlightContainer";
        {
            const title = document.createElement('h3');
            title.innerHTML = "Spotlight " +  (i*WIDTH + j+1);
            newDiv.appendChild(title);
        }
        {
            const newSpotlight = document.createElement('hex-color-picker');
            newSpotlight.className = "spotlight";
            newSpotlight.id = i*WIDTH + j+1;
            newDiv.appendChild(newSpotlight);
        }
        newRow.appendChild(newDiv);
    }
    container.appendChild(newRow);
}
//Settings for backlight option
const elements = document.getElementsByClassName('backlightOptions');
for(let i=0; i < elements.length ; i++){
    if(enableBacklight){
        elements[i].style.visibility = "visible";
        elements[i].style.height = "100%";
    }
}

const pickers = document.getElementsByTagName('hex-color-picker');
for(let i=0;i<pickers.length;i++){
    pickers[i].addEventListener('color-changed', (event) => {
        const newColor = event.detail.value;
        if(pickers[i].className==="spotlight"){
            updateClock("/spotlight" + pickers[i].id ,  pickers[i].color);
        }else{
            updateClock(pickers[i].className,pickers[i].color);
        }
    });
}

//Get settings once everything is initialized
sendData("/getsettings");