$(document).ready(function () {
    $("#TempsVapoActivate").click(function () {
        var valeur = $("#TempsVaporisation").val();
        $.post("TempsVaporisation", {
            TempsVaporisation: valeur
        });
    });
});

$(document).ready(function () {
    $("#TempsBrumiActivate").click(function () {
        var valeur = $("#TempsBrumisation").val();
        $.post("TempsBrumisation", {
            TempsBrumisation: valeur
        });
    });
});

$(document).ready(function () {
    $("#FrequenceVapoActivate").click(function () {
        var valeur = $("#FrequenceVaporisation").val();
        $.post("FrequenceVaporisation", {
            FrequenceVaporisation: valeur
        });
    });
});

$(document).ready(function () {
    $("#FrequenceBrumiActivate").click(function () {
        var valeur = $("#FrequenceBrumisation").val();
        $.post("FrequenceBrumisation", {
            FrequenceBrumisation: valeur
        });
    });
});

$(document).ready(function () {
    $("#IDtelegramActivate").click(function () {
        var valeur = $("#IDtelegram").val();
        $.post("IDtelegram", {
            IDtelegram: valeur
        });
    });
});

$(document).ready(function () {
    $("#TOKENtelegramActivate").click(function () {
        var valeur = $("#TOKENtelegram").val();
        $.post("TOKENtelegram", {
            TOKENtelegram: valeur
        });
    });
});

$(document).ready(function () {
    $("#appliquerchoixObjet1").click(function () {
        var valeur = $("#choixObjet1").val();
        $.post("choixObjet1", {
            choixObjet1: valeur
        });
    });
});


setInterval(function getData() {
    var xhttp = new XMLHttpRequest();

    xhttp.onreadystatechange = function () {
        if (this.readyState == 4 && this.status == 200) {
            document.getElementById("Temp_Thermo_4").innerHTML = this.responseText;
        }
    };

    xhttp.open("GET", "Temp_Thermo_4", true);
    xhttp.send();
}, 2000);

setInterval(function getData() {
    var xhttp = new XMLHttpRequest();

    xhttp.onreadystatechange = function () {
        if (this.readyState == 4 && this.status == 200) {
            document.getElementById("Temp_Thermo_5").innerHTML = this.responseText;
        }
    };

    xhttp.open("GET", "Temp_Thermo_5", true);
    xhttp.send();
}, 2000);

setInterval(function getData() {
    var xhttp = new XMLHttpRequest();

    xhttp.onreadystatechange = function () {
        if (this.readyState == 4 && this.status == 200) {
            document.getElementById("Temp_Thermo_32").innerHTML = this.responseText;
        }
    };

    xhttp.open("GET", "Temp_Thermo_32", true);
    xhttp.send();
}, 2000);

setInterval(function getData() {
    var xhttp = new XMLHttpRequest();

    xhttp.onreadystatechange = function () {
        if (this.readyState == 4 && this.status == 200) {
            document.getElementById("Temp_Thermo_33").innerHTML = this.responseText;
        }
    };

    xhttp.open("GET", "Temp_Thermo_33", true);
    xhttp.send();
}, 2000);

setInterval(function getData() {
    var xhttp = new XMLHttpRequest();

    xhttp.onreadystatechange = function () {
        if (this.readyState == 4 && this.status == 200) {
            document.getElementById("Humi_Thermo_4").innerHTML = this.responseText;
        }
    };

    xhttp.open("GET", "Humi_Thermo_4", true);
    xhttp.send();
}, 2000);

setInterval(function getData() {
    var xhttp = new XMLHttpRequest();

    xhttp.onreadystatechange = function () {
        if (this.readyState == 4 && this.status == 200) {
            document.getElementById("Humi_Thermo_5").innerHTML = this.responseText;
        }
    };

    xhttp.open("GET", "Humi_Thermo_5", true);
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

setInterval(function getData() {
    var xhttp = new XMLHttpRequest();

    xhttp.onreadystatechange = function () {
        if (this.readyState == 4 && this.status == 200) {
            document.getElementById("choixObjet1Affiche").innerHTML = this.responseText;
        }
    };

    xhttp.open("GET", "choixObjet1Affiche", true);
    xhttp.send();
}, 2000);


function Pompe_Activation() {
    var xhttp = new XMLHttpRequest();
    xhttp.open("GET", "Pompe_Activation", true);
    xhttp.send();
}




