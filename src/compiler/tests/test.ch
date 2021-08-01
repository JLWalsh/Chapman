#main() {
    val file = "out.txt";
    val answers = "";

    answers = answers + "name: " + prompt("What is your name?") + newline();
    answers = answers + "age: " + prompt("How old are you?") + newline();
    answers = answers + "favourite_colour: " + prompt("What is your favourite colour?") + newline();

    write(file, answers);
}