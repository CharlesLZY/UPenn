import React from 'react';
import { makeStyles } from '@mui/styles';
import { Paper } from '@mui/material';

import PostCard from './PostCard';

const useStyles = makeStyles({

  postSection: {
    margin: 25,
  },

  postSectionBG: {
    marginTop: 10,
    paddingTop: 10,
    paddingBottom: 10,
    width: 850,
  },

});

export default function PostBar({ posts }) {
  const classes = useStyles();

  function renderPosts(posts) {
    return (
      <Paper className={classes.postSectionBG}>
        <div className={classes.postSection}>
          {posts.map((post) => (
            <PostCard key={post._id} post={post} />
          ))}
        </div>
      </Paper>
    );
  }

  return (
    renderPosts(posts)
  );
}
