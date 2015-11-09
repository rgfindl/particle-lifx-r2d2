package com.bluefinengineering.iot;

import javax.servlet.ServletException;
import javax.servlet.http.HttpServlet;
import javax.servlet.http.HttpServletRequest;
import javax.servlet.http.HttpServletResponse;
import java.io.*;
import java.net.HttpURLConnection;
import java.net.URL;

/**
 * Created by randyfindley on 11/6/15.
 */
public class LifxProxyServlet extends HttpServlet {

    private static final String LIFX_HOST = "https://api.lifx.com";
    private static final String SET_STATE_PATH = "/v1/lights/%s/state";

    @Override
    protected void doGet(HttpServletRequest req, HttpServletResponse resp) throws ServletException, IOException {
        String key = req.getParameter("key");
        String selector = req.getParameter("selector");
        String color = req.getParameter("color");
        String duration = req.getParameter("duration");
        String brightness = req.getParameter("brightness");

        try {
            URL url = new URL(LIFX_HOST+String.format(SET_STATE_PATH, selector));
            HttpURLConnection connection = (HttpURLConnection) url.openConnection();
            connection.setRequestProperty("Authorization", "Bearer "+key);
            connection.setDoOutput(true);
            connection.setRequestMethod("PUT");

            OutputStreamWriter writer = new OutputStreamWriter(connection.getOutputStream());
            writer.write("{\"power\": \"on\", \"color\": \""+color+"\", \"brightness\": "+brightness+", \"duration\": \""+duration+"\"}");
            writer.close();

            BufferedReader reader = new BufferedReader(new InputStreamReader(connection.getInputStream()));
            String line;
            StringBuilder output = new StringBuilder();

            while ((line = reader.readLine()) != null) {
                output.append(line).append("\n");
            }
            reader.close();

            OutputStream os = resp.getOutputStream();
            os.write(output.toString().getBytes());
            resp.setStatus(200);
            return;

        } catch (Exception e) {

            OutputStream os = resp.getOutputStream();
            os.write(e.getMessage().getBytes());
            resp.setStatus(500);
        }
    }
}
