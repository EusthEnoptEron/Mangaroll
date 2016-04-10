/************************************************************************************

Filename    :   MainActivity.java
Content     :   
Created     :   
Authors     :   

Copyright   :   Copyright 2014 Oculus VR, LLC. All Rights reserved.


*************************************************************************************/
package ch.zomg.mangaroll;

import android.content.Intent;
import android.net.Uri;
import android.os.Bundle;
import android.os.Environment;
import android.util.Log;

import com.oculus.vrappframework.VrActivity;

import java.io.File;
import java.io.InputStream;
import java.net.HttpURLConnection;
import java.net.URL;
import java.util.ArrayList;
import java.util.List;
import java.util.regex.Pattern;

public class MainActivity extends VrActivity {
	public static final String TAG = "Mangaroll";
	public static final String USER_AGENT = "Mangaroll/1.0";

	private static final String[] ALLOWED_IMG_EXTS = new String[]{".jpg", ".png"};
	/** Load jni .so on initialization */
	static {
		Log.d(TAG, "LoadLibrary");
//        disableCertificateValidation();
		System.loadLibrary("mangaroll");
	}

	public static native long nativeSetAppInterface(VrActivity act, String fromPackageNameString, String commandString, String uriString);
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

	public String getConfigDir() {
		return getFilesDir().getAbsolutePath() + "/";
	}

	@Override
	protected void onCreate(Bundle savedInstanceState) {
		super.onCreate(savedInstanceState);


		Intent intent = getIntent();
		String commandString = VrActivity.getCommandStringFromIntent(intent);
		String fromPackageNameString = VrActivity.getPackageStringFromIntent(intent);
		String uriString = VrActivity.getUriStringFromIntent(intent);

		setAppPtr(nativeSetAppInterface(this, fromPackageNameString, commandString, uriString));
	}

	public static String InsertParam(String source, String paramName, String value) {
		return source.replaceAll(Pattern.quote(paramName), Uri.encode(value));
	}

//	public void StartSpeechRecognition() {
//		SpeechRecognizer speech = SpeechRecognizer.createSpeechRecognizer(getApplicationContext());
//		speech.setRecognitionListener(this);
//
//	}

	public static byte[] LoadHttpUrl(String str) {
		int totalLen = 0;
		byte[] returnBuffer = new byte[totalLen];

		Log.d(TAG, "LoadHttpUrl " + str);
		try {
			URL aURL = new URL(str);
			HttpURLConnection conn = (HttpURLConnection) aURL.openConnection();
			conn.setRequestProperty("User-Agent", USER_AGENT);
			conn.connect();

			InputStream is = conn.getInputStream();

			byte[] readbuffer = new byte[0x100000];

			while (true) {
				int count = is.read(readbuffer);
				if (count < 0) {
					break;
				}
				byte[] tempBuffer = new byte[totalLen + count];
				System.arraycopy(returnBuffer, 0, tempBuffer, 0, totalLen);
				System.arraycopy(readbuffer, 0, tempBuffer, totalLen, count);
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

	/**
	 * Loads the content of an archived file.
	 * Format: /^(rar|zip)://.+?[|].+$/
	 * @param path
	 * @return
	 */
	public static byte[] LoadArchivedFile(String path) {
		String type = path.substring(0, 3);
		int splitPos = path.indexOf("|");
		if(splitPos >= 0) {
			String archiveFile = path.substring(6, splitPos);
			String relativePath = path.substring(splitPos+1); // +1 skip pipe

			AbstractArchive archive;
			if(type.equals("rar")) {
				archive = new RarArchive(archiveFile);
			}
			else {
				archive = new ZipArchive(archiveFile);
			}
			return archive.getContent(relativePath);
		} else {
			return new byte[0];
		}
	}

	public static String[] GetZipFileList(String path) {
		AbstractArchive archive = new ZipArchive(path);
		return archive.getImageList();
	}

	public static String[] GetRarFileList(String path) {
		AbstractArchive archive = new RarArchive(path);
		return archive.getImageList();
	}


	public static String GetZipName(String path) {
		AbstractArchive archive = new ZipArchive(path);
		return archive.getName();
	}

	public static String GetRarName(String path) {
		AbstractArchive archive = new RarArchive(path);
		return archive.getName();
	}

	public String[] getAllStorageDirectories() {
		List<String> dirList = new ArrayList<>();

		dirList.add(getPrimaryExternalStorageRootDir(this));
		dirList.add(getSecondaryExternalStorageRootDir());

		return dirList.toArray(new String[dirList.size()]);
	}

//
//	public static String[] getAllStorageDirectories() {
//		final String state = Environment.getExternalStorageState();
//		final List<String> dirList = new ArrayList<>();
//
//		if ( Environment.MEDIA_MOUNTED.equals(state) || Environment.MEDIA_MOUNTED_READ_ONLY.equals(state) ) {  // we can read the External Storage...
//			//Retrieve the primary External Storage:
//			final File primaryExternalStorage = Environment.getExternalStorageDirectory();
//					//Retrieve the External Storages root directory:
//			final String externalStorageRootDir;
//			if ( (externalStorageRootDir = primaryExternalStorage.getParent()) == null ) {  // no parent...
//				dirList.add(primaryExternalStorage.getAbsolutePath() + "/");
//				Log.d(TAG, "External Storage: " + primaryExternalStorage + "\n");
//			}
//			else {
//				final File externalStorageRoot = new File( externalStorageRootDir );
//				final File[] files = externalStorageRoot.listFiles();
//
//				for ( final File file : files ) {
//					// Filter out legacy folder to prevent duplicate files
//					if ( file.isDirectory() && file.canRead() && !file.getAbsolutePath().endsWith("/legacy") && (file.listFiles().length > 0) ) {  // it is a real directory (not a USB drive)...
//						dirList.add(primaryExternalStorage.getAbsolutePath() + "/");
//						Log.d(TAG, "External Storage: " + file.getAbsolutePath() + "\n");
//					}
//				}
//			}
//		}
//		return dirList.toArray(new String[dirList.size()]);
//	}


	@Override
	public void onStart() {
		super.onStart();
	}

	@Override
	public void onStop() {
		super.onStop();
	}
}
