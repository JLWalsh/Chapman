#ask(file) {
    val answers = "";

    answers = answers + "name: " + prompt("What is your name?");
    answers = answers + "age: " + prompt("How old are you?");
    answers = answers + "favourite_colour: " + prompt("What is your favourite colour?"); 

    write(file, answers);
}