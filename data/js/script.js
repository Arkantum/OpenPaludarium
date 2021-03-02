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

var isEditing = false,
  tempNameValue = "",
  tempDataValue = "";

// Handles live/dynamic element events, i.e. for newly created edit buttons
$(document).on('click', '.edit', function() {
  var parentRow = $(this).closest('tr'),
    tableBody = parentRow.closest('tbody'),
    tdName = parentRow.children('td.name'),
    tdData = parentRow.children('td.data');

  if (isEditing) {
    var nameInput = tableBody.find('input[name="name"]'),
      dataInput = tableBody.find('input[name="data"]'),
      tdNameInput = nameInput.closest('td'),
      tdDataInput = dataInput.closest('td'),
      currentEdit = tdNameInput.parent().find('td.edit');

    if ($(this).is(currentEdit)) {
      // Save new values as static html
      var tdNameValue = nameInput.prop('value'),
        tdDataValue = dataInput.prop('value');

      tdNameInput.empty();
      tdDataInput.empty();

      tdNameInput.html(tdNameValue);
      tdDataInput.html(tdDataValue);
    } else {
      // Restore previous html values
      tdNameInput.empty();
      tdDataInput.empty();

      tdNameInput.html(tempNameValue);
      tdDataInput.html(tempDataValue);
    }
    
    
    // Display static row
    currentEdit.html('<i class="fa fa-pencil" aria-hidden="true"></i>');
    isEditing = false;
  } else {
    // Display editable input row
    isEditing = true;
    $(this).html('<i class="fa fa-floppy-o" aria-hidden="true"></i>');

    var tdNameValue = tdName.html(),
      tdDataValue = tdData.html();

    // Save current html values for canceling an edit
    tempNameValue = tdNameValue;
    tempDataValue = tdDataValue;

    // Remove existing html values
    tdName.empty();
    tdData.empty();

    // Create input forms
    tdName.html('<input type="text" name="name" value="' + tdNameValue + '">');
    tdData.html('<input type="text" name="data" value="' + tdDataValue + '">');
  }
});

// Handles live/dynamic element events, i.e. for newly created trash buttons
$(document).on('click', '.trash', function() {
  // Turn off editing if row is current input
  if (isEditing) {
    var parentRow = $(this).closest('tr'),
      tableBody = parentRow.closest('tbody'),
      tdInput = tableBody.find('input').closest('td'),
      currentEdit = tdInput.parent().find('td.edit'),
      thisEdit = parentRow.find('td.edit');

    if (thisEdit.is(thisEdit)) {
      isEditing = false;
    }
  }

  // Remove selected row from table
  $(this).closest('tr').remove();
});

$('.new-row').on('click', function() {
  var tableBody = $(this).closest('tbody'),
    trNew = '<tr><td class="name"><input type="text" name="name" value=""></td><td class="data"><input type="text" name="data" value=""></td><td class="edit"><i class="fa fa-floppy-o" aria-hidden="true"></i></td><td class="trash"><i class="fa fa-ban" aria-hidden="true"></i></td></tr>';

  if (isEditing) {
    var nameInput = tableBody.find('input[name="name"]'),
      dataInput = tableBody.find('input[name="data"]'),
      tdNameInput = nameInput.closest('td'),
      tdDataInput = dataInput.closest('td'),
      currentEdit = tdNameInput.parent().find('td.edit');

    // Get current input values for newly created input cases
    var newNameInput = nameInput.prop('value'),
      newDataInput = dataInput.prop('value');

    // Restore previous html values
    tdNameInput.empty();
    tdDataInput.empty();

    tdNameInput.html(newNameInput);
    tdDataInput.html(newDataInput);

    // Display static row
    currentEdit.html('Edit');
  }

  isEditing = true;
  tableBody.find('tr:last').before(trNew);
});
