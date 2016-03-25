package ch.zomg.mangaroll;

import android.util.Xml;

import org.apache.commons.io.FilenameUtils;
import org.xml.sax.InputSource;
import org.xml.sax.SAXException;
import org.xml.sax.XMLReader;
import org.xmlpull.v1.XmlPullParser;
import org.xmlpull.v1.XmlPullParserException;

import java.io.IOException;
import java.io.InputStream;
import java.io.InputStreamReader;
import java.io.Reader;
import java.net.ContentHandler;
import java.util.Objects;

import javax.xml.parsers.ParserConfigurationException;
import javax.xml.parsers.SAXParser;
import javax.xml.parsers.SAXParserFactory;

/**
 * Created by Eusthron on 2016/03/25.
 */
public abstract class AbstractArchive {
    String[] IMG_EXTENSIONS = new String[]{"jpg", "jpeg", "png", "tga", "bmp", "gif"};

    /**
     * Gets the list of images contained in the file.
     * Returns NULL if something goes wrong.
     * @return
     */
    public abstract String[] getImageList();

    /**
     * Gets the content of a file as byte array. Empty if it fails.
     * @param file
     * @return
     */
    public abstract byte[] getContent(String file);

    /**
     * Tries to find the name of a comic/manga (*.opf) and returns the file name without extensions if nothing is found.
     * This is not a full-fledged epub parser, because in that case we'd parse META-INF/container.xml.
     * @return
     */
    public abstract String getName();

    protected  String readOpf(InputStream in) throws XmlPullParserException, IOException {

        XmlPullParser parser = Xml.newPullParser();
        parser.setFeature(XmlPullParser.FEATURE_PROCESS_NAMESPACES, false);
        parser.setInput(in, null);

        while(parser.next() != XmlPullParser.END_DOCUMENT) {
            if(parser.getEventType() == XmlPullParser.START_TAG) {
                if("title".equals(parser.getName())) {
                    return parser.nextText();
                }
            }
        }

        return null;
    }

}
