package ch.zomg.mangaroll;

import android.util.Log;

import org.apache.commons.io.FilenameUtils;
import org.apache.commons.io.IOUtils;
import org.xmlpull.v1.XmlPullParserException;

import java.io.IOException;
import java.io.InputStream;
import java.util.ArrayList;
import java.util.Enumeration;
import java.util.List;
import java.util.zip.ZipEntry;
import java.util.zip.ZipFile;

/**
 * Created by Eusthron on 2016/03/25.
 */
public class ZipArchive extends AbstractArchive {
    private final String path;
    private static final String TAG = "ZipArchive";

    public ZipArchive(String path   ) {
        this.path = path;
    }

    @Override
    public String[] getImageList() {
        List<String> fileList = new ArrayList<String>();
        try(ZipFile zipFile = new ZipFile(path)) {
            Enumeration<? extends ZipEntry> entries = zipFile.entries();
            while (entries.hasMoreElements()) {
                ZipEntry zipEntry = entries.nextElement();
                if(!zipEntry.isDirectory()) {
                    String imgPath = zipEntry.getName();
                    if(FilenameUtils.isExtension(imgPath.toLowerCase(), IMG_EXTENSIONS)) {
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

    @Override
    public byte[] getContent(String relativePath) {
        Log.i(TAG, "Unpacking " + relativePath);
        try(ZipFile zipFile = new ZipFile(path)) {
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

    @Override
    public String getName() {
        try(ZipFile zipFile = new ZipFile(path)) {
            Enumeration<? extends ZipEntry> entries = zipFile.entries();
            while (entries.hasMoreElements()) {
                ZipEntry zipEntry = entries.nextElement();
                if(!zipEntry.isDirectory()) {
                    if(zipEntry.getName().toLowerCase().endsWith(".opf")) {
                        try(InputStream in = zipFile.getInputStream(zipEntry)) {
                            String name = readOpf(in);
                            if(name != null) {
                                return name;
                            }
                        } catch (XmlPullParserException e) {
                            Log.e(TAG, e.getMessage());
                        }
                        break;
                    }
                }
            }
        } catch (IOException e) {
            Log.e(TAG, e.getMessage());
        }
        return FilenameUtils.getBaseName(path);
    }
}
