#outer() {
    val x = "it works!!!";
    #inner() {
        print(x);
    }

    return inner;
}
#main() {
    val inner = outer();

    inner();
}

