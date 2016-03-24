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
import android.util.Log;

import com.oculus.vrappframework.VrActivity;
import junrar.Archive;
import junrar.exception.RarException;
import junrar.impl.FileVolumeManager;
import junrar.rarfile.FileHeader;

import org.apache.commons.io.IOUtils;

import java.io.ByteArrayOutputStream;
import java.io.File;
import java.io.IOException;
import java.io.InputStream;
import java.io.OutputStream;
import java.net.HttpURLConnection;
import java.net.URL;
import java.util.ArrayList;
import java.util.Enumeration;
import java.util.List;
import java.util.regex.Pattern;
import java.util.zip.ZipEntry;
import java.util.zip.ZipFile;

public class MainActivity extends VrActivity {
	public static final String TAG = "Mangaroll";
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
			conn.setRequestProperty("User-Agent", "Mangaroll/1.0");
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
	 * Gets the list of image files within an archive.
	 *
	 * @param path
	 * @return
	 */
	public static String[] GetArchiveFileList(String path) {

		if (path.endsWith(".cbz")) {
			return GetZipFileList(path);
		} else if (path.endsWith(".cbr")) {
			return GetRarFileList(path);
		}
		return null;
	}

	public static byte[] LoadArchivedFile(String path) {
		String type = path.substring(0, 3);
		int splitPos = path.indexOf(type.equals("rar") ? ".cbr" : ".cbz");
		if(splitPos >= 0) {
			splitPos += 4;
			String archiveFile = path.substring(6, splitPos);
			String relativePath = path.substring(splitPos+1); // +1 because of slash

			if(type.equals("rar")) {
				return unrar(archiveFile, relativePath);
			} else {
				return unzip(archiveFile, relativePath);
			}
		} else {
			return new byte[0];
		}
	}

	private static byte[] unzip(String archiveFile, String relativePath) {
		Log.i(TAG,"Unpacking "+ relativePath);
		try(ZipFile zipFile = new ZipFile(archiveFile)) {
			ZipEntry entry = zipFile.getEntry(relativePath);

			try(InputStream stream = zipFile.getInputStream(entry)) {
				byte[] file = IOUtils.toByteArray(stream);
				Log.i(TAG, "File is " + file.length + " bytes long!");
				return file;
			}
		} catch (IOException e) {
			Log.e(TAG, e.getMessage());
			return new byte[0];
		}
	}


	private static byte[] unrar(String archiveFile, String relativePath) {
		File f = new File(archiveFile);

		try(Archive a = new Archive(new FileVolumeManager(f));
				ByteArrayOutputStream output = new ByteArrayOutputStream() ) {
			FileHeader fh = a.nextFileHeader();
			while (fh != null) {
				String name = fh.getFileNameString().trim();
				if (name.equals(relativePath)) {
					a.extractFile(fh, output);
					return  output.toByteArray();
				}
				fh = a.nextFileHeader();
			}
		} catch (RarException e) {
			Log.e(TAG, e.getMessage());
		} catch (IOException e) {
			Log.e(TAG, e.getMessage());
		}
		return null;
	}


	private static String[] GetZipFileList(String path) {
		List<String> fileList = new ArrayList<String>();
		try(ZipFile zipFile = new ZipFile(path)) {
			Enumeration<? extends ZipEntry> entries = zipFile.entries();
			while (entries.hasMoreElements()) {
				ZipEntry zipEntry = entries.nextElement();
				if(!zipEntry.isDirectory()) {
					String imgPath = zipEntry.getName();
					if(imgPath.toLowerCase().endsWith(".jpg") || imgPath.toLowerCase().endsWith(".png")) {
						fileList.add(imgPath);
					}
				}
			}
		} catch (IOException e) {
			Log.e(TAG, e.getMessage());
			return null;
		}
		return fileList.toArray(new String[fileList.size()]);
	}

	private static String[] GetRarFileList(String path) {
		List<String> fileList = new ArrayList<String>();

		File f = new File(path);
		try(Archive a = new Archive(new FileVolumeManager(f))) {
			FileHeader fh = a.nextFileHeader();
			while(fh != null) {
				String name = fh.getFileNameString().trim();

				if(name.toLowerCase().endsWith(".jpg") || name.toLowerCase().endsWith(".png")) {
					fileList.add(fh.getFileNameString().trim());
				}

				fh = a.nextFileHeader();
			}
			return fileList.toArray(new String[fileList.size()]);
		} catch (RarException e) {
			Log.e(TAG, e.getMessage());
			return null;
		} catch (IOException e) {
			Log.e(TAG, e.getMessage());
			return null;
		}
	}



	@Override
	public void onStart() {
		super.onStart();
	}

	@Override
	public void onStop() {
		super.onStop();
	}
}
