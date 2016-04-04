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

    private Descriptor descriptor;

    public MangaFetcher(Descriptor descriptor) {
        this.descriptor = descriptor;
    }
    private static final String TAG = MangaFetcher.class.getName();

    public String[] fetch() {
        try {
            setHasMore(false);

            Log.i(TAG, descriptor.getUrl());
            Document doc = Jsoup.connect(descriptor.getUrl()).userAgent(MainActivity.USER_AGENT).get();
            URI uri = URI.create(descriptor.getUrl());

            List<String> links = new ArrayList<>();

            for(Element img : doc.select(descriptor.getItemSelector())) {
                String result = img.attr("src");
                if(result.isEmpty()) {
                    result = img.attr("href");
                }
                if(!result.isEmpty()) {
                    result = uri.resolve(result).toString();

                    if(descriptor.getHandler() != null) {
                        Descriptor childDescriptor = descriptor.getHandler().clone();
                        childDescriptor.setUrl(result);
                        MangaFetcher child = new MangaFetcher(childDescriptor);
                        links.addAll(Arrays.asList(child.fetch()));

                    } else {
                        links.add(result);
                    }
                }
            }


            if(links.size() == 0) {
                setHasMore(false);
            } else if(descriptor.getNextPageSelector() != null) {
                Elements nextPageLink = doc.select(descriptor.getNextPageSelector());
                setHasMore(nextPageLink.size() > 0 && !nextPageLink.get(0).attr("href").isEmpty());
                if(hasMore()) {
                    descriptor.setUrl(uri.resolve(nextPageLink.get(0).attr("href")).toString());
                }
            }

            return links.toArray(new String[links.size()]);
        } catch (IOException e) {
            Log.e(TAG, e.getMessage());
            return new String[0];
        }
    }

    @Override
    public boolean isContainerProvider() {
        return false;
    }
}
