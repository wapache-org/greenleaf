<template>
  <div id="app" class="d-flex flex-column h-100">
    <router-view></router-view>
    <div class="navbar" style="padding: .2rem 1rem;">
      <router-link tag="a" class="btn btn-secondary ml-auto" to="/" style="padding: 0 .75rem;">
        Home
      </router-link>
      <router-link tag="a" class="btn btn-link" to="about" style="padding: 0 .75rem;">
        About
      </router-link>
    </div>
  </div>
</template>

<script lang="ts">
import { Component, Vue } from 'vue-property-decorator';
export const planData: any[] = ['', ''];

@Component
export default class App extends Vue {}
</script>
