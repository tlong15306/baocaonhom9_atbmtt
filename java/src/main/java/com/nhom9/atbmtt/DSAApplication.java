package com.nhom9.atbmtt;

import javafx.application.Application;
import javafx.fxml.FXMLLoader;
import javafx.scene.Parent;
import javafx.scene.Scene;
import javafx.scene.image.Image;
import javafx.stage.Stage;

/**
 * Lớp chính của ứng dụng JavaFX
 * Khởi tạo giao diện và hiển thị cửa sổ chính
 *
 * @author Nhóm 9 - ATBMTT (Long & Vi)
 */
public class DSAApplication extends Application {

    @Override
    public void start(Stage primaryStage) throws Exception {
        // Tải giao diện FXML
        FXMLLoader loader = new FXMLLoader(getClass().getResource("/fxml/main_view.fxml"));
        Parent root = loader.load();

        // Thiết lập Scene
        Scene scene = new Scene(root, 1050, 780);
        scene.getStylesheets().add(getClass().getResource("/css/style.css").toExternalForm());

        // Thiết lập Stage
        primaryStage.setTitle("DSA Digital Signature — Nhóm 9 ATBMTT");
        primaryStage.setScene(scene);
        primaryStage.setMinWidth(960);
        primaryStage.setMinHeight(720);
        primaryStage.show();
    }

    public static void main(String[] args) {
        launch(args);
    }
}
