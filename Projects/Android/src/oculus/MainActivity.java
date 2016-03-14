/************************************************************************************

Filename    :   MainActivity.java
Content     :   
Created     :   
Authors     :   

Copyright   :   Copyright 2014 Oculus VR, LLC. All Rights reserved.


*************************************************************************************/
package oculus;

import android.content.Context;
import android.net.Uri;
import android.os.Bundle;
import android.speech.SpeechRecognizer;
import android.util.Log;
import android.content.Intent;
import com.oculus.vrappframework.VrActivity;

import java.io.InputStream;
import java.net.HttpURLConnection;
import java.net.URL;
import java.net.URLDecoder;
import java.security.SecureRandom;
import java.util.regex.Pattern;

import javax.net.ssl.HostnameVerifier;
import javax.net.ssl.HttpsURLConnection;
import javax.net.ssl.SSLContext;
import javax.net.ssl.SSLSession;
import javax.net.ssl.TrustManager;
import javax.net.ssl.X509TrustManager;
import java.security.cert.X509Certificate;

public class MainActivity extends VrActivity {
	public static final String TAG = "Mangaroll";

	/** Load jni .so on initialization */
	static {
		Log.d(TAG, "LoadLibrary");
		try {
			Thread.sleep(2000);
		} catch (InterruptedException e) {
			e.printStackTrace();
		}
//        disableCertificateValidation();
		System.loadLibrary("mangaroll");
	}

    public static native long nativeSetAppInterface( VrActivity act, String fromPackageNameString, String commandString, String uriString );
//
//    public static void disableCertificateValidation() {
//        // Create a trust manager that does not validate certificate chains
//        TrustManager[] trustAllCerts = new TrustManager[] {
//                new X509TrustManager() {
//                    public X509Certificate[] getAcceptedIssuers() {
//                        return new X509Certificate[0];
//                    }
//                    public void checkClientTrusted(X509Certificate[] certs, String authType) {}
//                    public void checkServerTrusted(X509Certificate[] certs, String authType) {}
//                }};
//
//        // Ignore differences between given hostname and certificate hostname
//        HostnameVerifier hv = new HostnameVerifier() {
//            public boolean verify(String hostname, SSLSession session) { return true; }
//        };
//
//        // Install the all-trusting trust manager
//        try {
//            SSLContext sc = SSLContext.getInstance("SSL");
//            sc.init(null, trustAllCerts, new SecureRandom());
//            HttpsURLConnection.setDefaultSSLSocketFactory(sc.getSocketFactory());
//            HttpsURLConnection.setDefaultHostnameVerifier(hv);
//        } catch (Exception e) {}
//    }

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
		Intent intent = getIntent();
		String commandString = VrActivity.getCommandStringFromIntent( intent );
		String fromPackageNameString = VrActivity.getPackageStringFromIntent( intent );
		String uriString = VrActivity.getUriStringFromIntent( intent );

		setAppPtr(nativeSetAppInterface(this, fromPackageNameString, commandString, uriString));
    }

	public static String InsertParam( String source, String paramName, String value) {
        return source.replaceAll(Pattern.quote(paramName), Uri.encode(value));
	}

//	public void StartSpeechRecognition() {
//		SpeechRecognizer speech = SpeechRecognizer.createSpeechRecognizer(getApplicationContext());
//		speech.setRecognitionListener(this);
//
//	}

	public static byte[] LoadHttpUrl( String str ) {
		int totalLen = 0;
		byte[] returnBuffer = new byte[totalLen];

		Log.d(TAG, "LoadHttpUrl " + str );
		try {
            URL aURL = new URL( str );
            HttpURLConnection conn = (HttpURLConnection)aURL.openConnection();
			conn.setRequestProperty("User-Agent", "Mangaroll/1.0");
            conn.connect();

			InputStream is = conn.getInputStream();

			byte[]	readbuffer = new byte[0x100000];

			while( true ) {
				int count = is.read(readbuffer);
				if ( count < 0 ) {
					break;
				}
				byte[] tempBuffer = new byte[totalLen + count];
				System.arraycopy(returnBuffer,  0,  tempBuffer,  0,  totalLen );
				System.arraycopy(readbuffer,  0, tempBuffer,  totalLen,  count );
				totalLen += count;
				returnBuffer = tempBuffer;
			}

			is.close();
            Log.w(TAG, "LoadHttpUrl success!");

        } catch (Exception e) {
			Log.w(TAG, "LoadHttpUrl", e);
		}

		Log.d(TAG, "totalLen " + totalLen);

		return returnBuffer;
	}
}
