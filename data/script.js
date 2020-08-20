

setInterval(function getData() {
    var xhttp = new XMLHttpRequest();

    xhttp.onreadystatechange = function () {
        if (this.readyState == 4 && this.status == 200) {
            document.getElementById("Temp_Thermo_16").innerHTML = this.responseText;
        }
    };

    xhttp.open("GET", "Temp_Thermo_16", true);
    xhttp.send();
}, 2000);

setInterval(function getData() {
    var xhttp = new XMLHttpRequest();
    
    xhttp.onreadystatechange = function () {
        if (this.readyState == 4 && this.status == 200) {
            document.getElementById("Temp_Thermo_36").innerHTML = this.responseText;
        }
    };

    xhttp.open("GET", "Temp_Thermo_36", true);
    xhttp.send();
}, 2000);

setInterval(function getData() {
    var xhttp = new XMLHttpRequest();
    
    xhttp.onreadystatechange = function () {
        if (this.readyState == 4 && this.status == 200) {
            document.getElementById("Temp_Thermo_17").innerHTML = this.responseText;
        }
    };

    xhttp.open("GET", "Temp_Thermo_17", true);
    xhttp.send();
}, 2000);

setInterval(function getData() {
    var xhttp = new XMLHttpRequest();
    
    xhttp.onreadystatechange = function () {
        if (this.readyState == 4 && this.status == 200) {
            document.getElementById("Temp_Thermo_35").innerHTML = this.responseText;
        }
    };

    xhttp.open("GET", "Temp_Thermo_35", true);
    xhttp.send();
}, 2000);

setInterval(function getData() {
    var xhttp = new XMLHttpRequest();

    xhttp.onreadystatechange = function () {
        if (this.readyState == 4 && this.status == 200) {
            document.getElementById("Humi_Thermo_16").innerHTML = this.responseText;
        }
    };

    xhttp.open("GET", "Humi_Thermo_16", true);
    xhttp.send();
}, 2000);

setInterval(function getData() {
    var xhttp = new XMLHttpRequest();

    xhttp.onreadystatechange = function () {
        if (this.readyState == 4 && this.status == 200) {
            document.getElementById("Humi_Thermo_36").innerHTML = this.responseText;
        }
    };

    xhttp.open("GET", "Humi_Thermo_36", true);
    xhttp.send();
}, 2000);


setInterval(function getData() {
    var xhttp = new XMLHttpRequest();

    xhttp.onreadystatechange = function () {
        if (this.readyState == 4 && this.status == 200) {
            document.getElementById("Temps").innerHTML = this.responseText;
        }
    };

    xhttp.open("GET", "Temps", true);
    xhttp.send();
}, 1000);


function Rampe_Eclairage_On() {
    var xhttp = new XMLHttpRequest();
    xhttp.open("GET", "Rampe_Eclairage_On", true);
    xhttp.send();
}

function Rampe_Eclairage_Off() {
    var xhttp = new XMLHttpRequest();
    xhttp.open("GET", "Rampe_Eclairage_Off", true);
    xhttp.send();
}

function Pompe_Activation() {
    var xhttp = new XMLHttpRequest();
    xhttp.open("GET", "Pompe_Activation", true);
    xhttp.send();
}