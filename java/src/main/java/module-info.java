module com.nhom9.atbmtt {
    requires javafx.controls;
    requires javafx.fxml;
    requires javafx.graphics;
    
    opens com.nhom9.atbmtt to javafx.fxml;
    exports com.nhom9.atbmtt;
}
