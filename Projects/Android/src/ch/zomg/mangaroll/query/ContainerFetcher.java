package ch.zomg.mangaroll.query;

import android.net.Uri;
import android.util.Log;

import com.google.gson.Gson;

import org.jsoup.Jsoup;
import org.jsoup.nodes.Document;
import org.jsoup.nodes.Element;
import org.jsoup.select.Elements;

import java.io.IOException;
import java.net.URI;
import java.util.ArrayList;
import java.util.List;

/**
 * Created by Simon on 2016/03/30.
 */
public class ContainerFetcher extends Fetcher {
    private Descriptor descriptor;

    public ContainerFetcher(Descriptor descriptor) {
        this.descriptor = descriptor;
    }
    private static final String TAG = ContainerFetcher.class.getName();

    public Fetcher[] fetch() {
        try {
            Log.i(TAG, descriptor.getUrl());
            setHasMore(false);
            Document doc = Jsoup.connect(descriptor.getUrl()).get();
            URI uri = URI.create(descriptor.getUrl());

            List<Fetcher> fetchers = new ArrayList<>();

            // Search for items
            for(Element element : doc.select(descriptor.getItemSelector())) {
                String link = element.select(descriptor.getLinkSelector()).attr("href");
                String name = element.select(descriptor.getNameSelector()).text();
//                String thumb = element.select(descriptor.getThumbSelector()).attr("src");

                if(!link.isEmpty()) {
                    Descriptor childDescriptor = descriptor.getHandler().clone();
                    childDescriptor.setUrl(uri.resolve(link).toString());

                    if (childDescriptor.getType() == Descriptor.Type.CONTAINER) {
                        ContainerFetcher fetcher = new ContainerFetcher(childDescriptor);
                        fetcher.setName(name);
                        fetchers.add(fetcher);
                    } else {
                        MangaFetcher fetcher = new MangaFetcher(childDescriptor);
                        fetcher.setName(name);
                        fetchers.add(fetcher);
                    }
                }
            }

            // Determine if there is more
            if(descriptor.getNextPageSelector() != null) {
                Elements nextPageLink = doc.select(descriptor.getNextPageSelector());
                setHasMore(nextPageLink.size() > 0);
                if(hasMore()) {
                    descriptor.setUrl(uri.resolve(nextPageLink.get(0).attr("href")).toString());
                }
            } else {
                setHasMore(false);
            }

            return fetchers.toArray(new Fetcher[fetchers.size()]);
        } catch (IOException e) {
            return new Fetcher[0];
        }
    }

    @Override
    public boolean isContainerProvider() {
        return true;
    }
}
