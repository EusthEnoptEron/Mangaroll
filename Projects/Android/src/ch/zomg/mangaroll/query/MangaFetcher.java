package ch.zomg.mangaroll.query;

import android.util.Log;

import org.jsoup.Jsoup;
import org.jsoup.nodes.Document;
import org.jsoup.nodes.Element;
import org.jsoup.select.Elements;

import java.io.IOException;
import java.net.URI;
import java.util.ArrayList;
import java.util.Arrays;
import java.util.List;

import ch.zomg.mangaroll.MainActivity;

/**
 * Created by Simon on 2016/03/30.
 */
public class MangaFetcher extends Fetcher {
    public MangaFetcher(Descriptor descriptor) {
        super(descriptor);
    }


    @Override
    protected boolean isDescriptorValid() {
        return descriptor.getItemSelector() != null;
    }

    private static final String TAG = MangaFetcher.class.getName();

    public String[] fetch() {
        try {
            Log.i(TAG, descriptor.getUrl());
            List<String> links = new ArrayList<>();

            for (Element img : next()) {
                String result = extractURL(img);
                if (!result.isEmpty()) {
                    Log.i(TAG, "Found image: " + result);
                    if (descriptor.getHandler() != null) {
                        Descriptor childDescriptor = descriptor.getHandler().clone();
                        childDescriptor.setUrl(result);
                        MangaFetcher child = new MangaFetcher(childDescriptor);
                        links.addAll(Arrays.asList(child.fetch()));

                    } else {
                        links.add(result);
                    }
                } else {
                    Log.w(TAG, "Result was empty! SRC: " + img.attr("src"));
                }
            }

            return links.toArray(new String[links.size()]);
        } catch(Exception e) {
            Log.e(TAG, e.getMessage());
            return new String[0];
        }
    }

    @Override
    public boolean isContainerProvider() {
        return false;
    }
}
