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

import ch.zomg.mangaroll.MainActivity;

/**
 * Created by Simon on 2016/03/30.
 */
public class ContainerFetcher extends Fetcher {
    private static final String TAG = ContainerFetcher.class.getName();

    public ContainerFetcher(Descriptor descriptor) {
        super(descriptor);
    }

    @Override
    protected boolean isDescriptorValid() {
        return descriptor.getItemSelector() != null && descriptor.getLinkSelector() != null && descriptor.getHandler() != null && descriptor.getUrl() != null && descriptor.getNameSelector() != null;
    }

    public Fetcher[] fetch() {
        try {
            Log.i(TAG, descriptor.getUrl());

            List<Fetcher> fetchers = new ArrayList<>();

            Log.i(TAG, "Item selector: "+descriptor.getItemSelector());

            // Search for items
            for(Element element : next()) {
                Log.i(TAG, "Analyzing item");
                String link = extractURL(element.select(descriptor.getLinkSelector()));
                String name = extractName(element.select(descriptor.getNameSelector()));
                String thumb = "";
                if(descriptor.getThumbSelector() != null) {
                    thumb = extractURL(element.select(descriptor.getThumbSelector()));
                }

                if(!link.isEmpty()) {
                    Log.i(TAG, "Link OK");
                    Descriptor childDescriptor = descriptor.getHandler().clone();
                    childDescriptor.setUrl(base.resolve(link).toString());

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

            return fetchers.toArray(new Fetcher[fetchers.size()]);
        } catch (Exception e) {
            Log.e(TAG, e.getMessage());
            return new Fetcher[0];
        }
    }

    @Override
    public boolean isContainerProvider() {
        return true;
    }
}
