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
            if(descriptor.getLinkSelector() == null || descriptor.getHandler() == null || descriptor.getUrl() == null || descriptor.getNameSelector() == null) {
                throw new RuntimeException("Not all mandatory fields are provided!");
            }

            Log.i(TAG, descriptor.getUrl());
            setHasMore(false);
            Document doc = Jsoup.connect(descriptor.getUrl()).get();
            URI uri = URI.create(descriptor.getUrl());

            List<Fetcher> fetchers = new ArrayList<>();

            Log.i(TAG, "Item selector: "+descriptor.getItemSelector());
            // Search for items
            for(Element element : doc.select(descriptor.getItemSelector())) {
                Log.i(TAG, "Analyzing item");
                String link = extractURL(uri, element.select(descriptor.getLinkSelector()));
                String name = extractName(element.select(descriptor.getNameSelector()));
                String thumb = "";
                if(descriptor.getThumbSelector() != null) {
                    thumb = extractURL(uri, element.select(descriptor.getThumbSelector()));
                }

                if(!link.isEmpty()) {
                    Log.i(TAG, "Link OK");
                    Descriptor childDescriptor = descriptor.getHandler().clone();
                    childDescriptor.setUrl(uri.resolve(link).toString());

                    if (childDescriptor.getType() == Descriptor.Type.CONTAINER) {
                        Log.i(TAG, "Is another container");
                        ContainerFetcher fetcher = new ContainerFetcher(childDescriptor);
                        fetcher.setName(name);
                        fetcher.setThumb(thumb);
                        fetchers.add(fetcher);
                    } else {
                        Log.i(TAG, "Is a manga");
                        MangaFetcher fetcher = new MangaFetcher(childDescriptor);
                        fetcher.setName(name);
                        fetcher.setThumb(thumb);
                        fetchers.add(fetcher);
                    }
                }
                Log.i(TAG, "Done");
            }
            Log.i(TAG, "Done with this container");


            // Determine if there is more
            if(descriptor.getNextPageSelector() != null) {
                Elements nextPageLink = doc.select(descriptor.getNextPageSelector());
                setHasMore(nextPageLink.size() > 0 && !nextPageLink.get(0).attr("href").isEmpty());
                if(hasMore()) {
                    descriptor.setUrl(uri.resolve(nextPageLink.get(0).attr("href")).toString());
                }
            }

            return fetchers.toArray(new Fetcher[fetchers.size()]);
        } catch (Exception e) {
            Log.e(TAG, e.getMessage());
            setHasMore(false);
            return new Fetcher[0];
        }
    }

    @Override
    public boolean isContainerProvider() {
        return true;
    }
}
