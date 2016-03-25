package ch.zomg.mangaroll;

import android.util.Log;

import org.apache.commons.io.FilenameUtils;
import org.xmlpull.v1.XmlPullParserException;

import java.io.ByteArrayOutputStream;
import java.io.File;
import java.io.IOException;
import java.io.InputStream;
import java.util.ArrayList;
import java.util.List;

import junrar.Archive;
import junrar.exception.RarException;
import junrar.impl.FileVolumeManager;
import junrar.rarfile.FileHeader;

/**
 * Created by Eusthron on 2016/03/25.
 */
public class RarArchive extends AbstractArchive {
    private final static String TAG = "RarArchive";
    private final String path;

    public RarArchive(String path) {
        this.path = path;
    }

    @Override
    public String[] getImageList() {
        List<String> fileList = new ArrayList<String>();

        File f = new File(path);
        try(Archive a = new Archive(new FileVolumeManager(f))) {
            FileHeader fh = a.nextFileHeader();
            while(fh != null) {
                String name = fh.getFileNameString().trim();

                if(FilenameUtils.isExtension(name.toLowerCase(), IMG_EXTENSIONS)) {
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
    public byte[] getContent(String relativePath) {
        File f = new File(path);

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

    @Override
    public String getName() {
        File f = new File(path);
        try(Archive a = new Archive(new FileVolumeManager(f))) {
            FileHeader fh = a.nextFileHeader();
            while(fh != null) {
                if(fh.getFileNameString().trim().toLowerCase().endsWith(".opf")) {
                    try(InputStream in = a.getInputStream(fh)) {
                        String name = readOpf(in);
                        if(name != null) {
                            return name;
                        }
                    } catch (XmlPullParserException e) {
                        Log.e(TAG, e.getMessage());
                    }
                    break;
                }
                fh = a.nextFileHeader();
            }
        } catch (RarException e) {
            Log.e(TAG, e.getMessage());
        } catch (IOException e) {
            Log.e(TAG, e.getMessage());
        }

        return FilenameUtils.getBaseName(path);
    }
}
