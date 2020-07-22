$(document).ready(function(){
    $("#appliquerTempsEntre").click(function(){
        var valueTempsVapoParJours = $("#choixValeurVapoParJours").val();
        $.post("VapoParJours",{
            ValeurVapoParJours: valueTempsVapoParJours
        });
    });
});

setInterval(function getData() {
    var xhttp = new XMLHttpRequest();

    xhttp.onreadystatechange = function () {
        if (this.readyState == 4 && this.status == 200) {
            document.getElementById("valeurTemp").innerHTML = this.responseText;
        }
    };

    xhttp.open("GET", "lireTemp", true);
    xhttp.send();
}, 2000);

setInterval(function getData() {
    var xhttp = new XMLHttpRequest();

    xhttp.onreadystatechange = function () {
        if (this.readyState == 4 && this.status == 200) {
            document.getElementById("valeurTempEau").innerHTML = this.responseText;
        }
    };

    xhttp.open("GET", "lireTempEau", true);
    xhttp.send();
}, 1000);

setInterval(function getData() {
    var xhttp = new XMLHttpRequest();

    xhttp.onreadystatechange = function () {
        if (this.readyState == 4 && this.status == 200) {
            document.getElementById("valeurTempMoitie").innerHTML = this.responseText;
        }
    };

    xhttp.open("GET", "lireTempMoitie", true);
    xhttp.send();
}, 1000);

setInterval(function getData() {
    var xhttp = new XMLHttpRequest();

    xhttp.onreadystatechange = function () {
        if (this.readyState == 4 && this.status == 200) {
            document.getElementById("valeurHumi").innerHTML = this.responseText;
        }
    };

    xhttp.open("GET", "lireHumi", true);
    xhttp.send();
}, 2000);

setInterval(function getData() {
    var xhttp = new XMLHttpRequest();

    xhttp.onreadystatechange = function () {
        if (this.readyState == 4 && this.status == 200) {
            document.getElementById("valeurHumiMoitie").innerHTML = this.responseText;
        }
    };

    xhttp.open("GET", "lireHumiMoitie", true);
    xhttp.send();
}, 1000);

setInterval(function getData() {
    var xhttp = new XMLHttpRequest();

    xhttp.onreadystatechange = function () {
        if (this.readyState == 4 && this.status == 200) {
            document.getElementById("valeurTemps").innerHTML = this.responseText;
        }
    };

    xhttp.open("GET", "lireTemps", true);
    xhttp.send();
}, 1000);


function onButtonBarre() {
    var xhttp = new XMLHttpRequest();
    xhttp.open("GET", "onBarre", true);
    xhttp.send();
}

function offButtonBarre() {
    var xhttp = new XMLHttpRequest();
    xhttp.open("GET", "offBarre", true);
    xhttp.send();
}

function onButtonPompe() {
    var xhttp = new XMLHttpRequest();
    xhttp.open("GET", "onPompe", true);
    xhttp.send();
}