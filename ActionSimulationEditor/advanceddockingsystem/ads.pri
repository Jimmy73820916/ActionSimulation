CONFIG(debug, debug|release){
    LIBS += -lqtadvanceddockingd
}
else{
    LIBS += -lqtadvanceddocking
}


