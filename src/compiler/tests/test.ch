val z = 20;

#main() {
    val y = 32;
    #addTwo(x) {
        return x + 2;
    };

    val z = otherFunction(addTwo(y));

    print(z);
}

#otherFunction(a) {
    val z = 40;

    return 40 + 2 + a;
}