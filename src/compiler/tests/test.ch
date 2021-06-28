#outer() {
    val x = "it works!!!";
    #first() {
        print(x);
    }
    x = "doot doot";

    #second() {
        print("second: " + x);       
    }

    second();
    x = "hello";
    #third() {
        print("third: " + x);       
    }
    third();
    
    return first;
}
#main() {
    val inner = outer();

    inner();
}

